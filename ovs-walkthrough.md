# Code Walkthrough Notes

## Some Data Structures

`struct udpif` is basically a upcall handler context that has information about the set of `handlers` that handle the upcalls, set of `revalidators` that handle the flow updation in the fast path, `dpif` the datapath handle, `dump_seq` for dumping all the flows and `reval_seq` for controlling the revalidation, `ukeys` that contain the hash of flow keys which are installed in the fast path and other flow related statistics such as `flow_limit`, `n_flows`, `max_n_flows` etc.

`struct ofpbuf` is a descriptor of a buffer which is gotten through some means (from heap or stack etc.). It contains various pointers into the buffer such as `base`, `data`, `header`, `msg` etc and its capacity in `size, allocated`. It can be made to point to some memory area by calling `ofpbuf_use_stub`, for example.

`struct dpif_upcall` contains information about a packet sent from the data path (be it kernel or DPDK). It contains the `packet` itself, `key`, a set of netlink attributes that describe various packet metadata (like input port or tunnel etc.), `ufid` the unique identifier for a flow (in case of revalidation) etc.

`struct upcall` contains the context needed for installing a flow in the data path. It contains `flow` which represents the packet's needed parameters, `in_port` from which the packet came in, `xout` result of translating actions, `odp_actions` a set of data path actions, `wc` the wildcard masks that are accumulated by walking through the ofproto tables, `dump_seq` and `reval_seq` the values of the global sequence number cached for later comparison etc.

## Some Function Walkthroughs

### recv_upcalls

All the needed storage to handle miss upcalls is created on the stack:

```c
    struct udpif *udpif = handler->udpif;
    uint64_t recv_stubs[UPCALL_MAX_BATCH][512 / 8];
    struct ofpbuf recv_bufs[UPCALL_MAX_BATCH];
    struct dpif_upcall dupcalls[UPCALL_MAX_BATCH];
    struct upcall upcalls[UPCALL_MAX_BATCH];
    struct flow flows[UPCALL_MAX_BATCH];
    size_t n_upcalls, i;
```

At a time, `recv_upcalls` handles 64 miss upcalls. `recv_bufs` are buffer descriptors which point to `recv_stubs` for temporary storage. `dpif_recv` is used to retrieve an upcall from the data path into `dupcalls`:

```c
        if (dpif_recv(udpif->dpif, handler->handler_id, dupcall, recv_buf)) {
            break;
        }
```

`dpif_recv` fills the `dupcall` with necessary information by storing any packet data in `recv_buf`. Then, once the related information is extracted in `dupcall` like the packet and netlink attributes, they are converted to a `flow`:

```c
        if (odp_flow_key_to_flow(dupcall->key, dupcall->key_len, flow)
            == ODP_FIT_ERROR) {
            goto free_dupcall;
        }
```

Then the whole upcall structure is initialized with `upcall_receive`:

```c
        error = upcall_receive(upcall, udpif->backer, p_packet,
                               dupcall->type, dupcall->userdata, flow, mru,
                               &dupcall->ufid, dupcall->pmd_thread_id);
```

Among things of interest, `in_port` and `ofproto` in upcall need to be populated to identify the bridge and the port in which the packet came. All other fields of `upcall` stricture are initialized. In essence, we are translating information from `dupcall` to `upcall`. In case, no `in_port` or `ofproto` is found, then a flow is installed with a drop action - since we have all the netlink attributes in `dupcall`, it is used to install the flow. The `actions` passed is NULL so it will be a drop:

```c
                dpif_flow_put(udpif->dpif, DPIF_FP_CREATE, dupcall->key,
                              dupcall->key_len, NULL, 0, NULL, 0,
                              &dupcall->ufid, PMD_ID_NULL, NULL);
```

Then we extract packet fields into the `flow` (earlier we translated netlink attributes to `flow` with `odp_flow_key_to_flow` and finally process the upcall:

```c
        error = process_upcall(udpif, upcall,
                               &upcall->odp_actions, &upcall->wc);
        if (error) {
            goto cleanup;
        }
```

This takes care of walking through the ofproto tables and constructing the megaflow and the actions needed. The actual job of installing the flows into the data path is taken care of by `handle_upcalls`:

```c
    if (n_upcalls) {
        handle_upcalls(handler->udpif, upcalls, n_upcalls);
        for (i = 0; i < n_upcalls; i++) {
            dp_packet_uninit((struct dp_packet *)upcalls[i].packet);
            ofpbuf_uninit(&recv_bufs[i]);
            upcall_uninit(&upcalls[i]);
        }
    }
```

After installing the flows, the allocated structures are freed.

### dpif_recv

The intial uplifting of getting the information about an upcall from the fast path is done by `dpif_recv`.

```c
    if (dpif->dpif_class->recv) {
        error = dpif->dpif_class->recv(dpif, handler_id, upcall, buf);
    }
```

As we can see, this is just a wrapper around the fast path specific receive function. For kernel as fast path, the function that is called is `dpif_netlink_recv`

### dpif_netlink_recv__

This function which is called by `dpif_netlink_recv` (non-Windows case), basically 

```c
    handler = &dpif->handlers[handler_id];
    if (handler->event_offset >= handler->n_events) {
        handler->event_offset = handler->n_events = 0;

            retval = epoll_wait(handler->epoll_fd, handler->epoll_events,
                                dpif->uc_array_size, 0);

            handler->n_events = retval;

    }
```

Every handler thread is associated with a netlink socket (the paper says the upcalls are distributed across netlink sockets - and there is one socket per upcall handler). The above snippet simply tries for `epoll` to get information about any events queued for this epoll socket and stores the number of events.

```c
while (handler->event_offset < handler->n_events) {
        int idx = handler->epoll_events[handler->event_offset].data.u32;
        struct dpif_channel *ch = &dpif->handlers[handler_id].channels[idx];

          <snip>

          error = nl_sock_recv(ch->sock, buf, false);
          
          <snip>
          
          error = parse_odp_packet(dpif, buf, upcall, &dp_ifindex);
}
```

If there are events registered on epoll, then the information is received into the `buf` and then the packet is parsed from the `buf` through `parse_odp_packet`

### parse_odp_packet

`parse_odp_packet` is expecting a well known set of netlink attributes and simply takes them and populates the `dpif_upcall` structure. The first one that is tried for extraction is `ovs_header`:

```c
    nlmsg = ofpbuf_try_pull(&b, sizeof *nlmsg);
    genl = ofpbuf_try_pull(&b, sizeof *genl);
    ovs_header = ofpbuf_try_pull(&b, sizeof *ovs_header);
```

The `ofpbuf_try_pull` is similar to kernel's `skb_try_pull` where it tries to extract a portion of data and advances the buffer. Then, the remaining buffer is validated against a known `ovs_packet_policy` which has a set of netlink attributes defined in sequence.

```c
    if (!nlmsg || !genl || !ovs_header
        || nlmsg->nlmsg_type != ovs_packet_family
        || !nl_policy_parse(&b, 0, ovs_packet_policy, a,
                            ARRAY_SIZE(ovs_packet_policy))) {
        return EINVAL;
    }
```

When the buffer `b` is walked through for entries in `ovs_packet_policy`, `a` (an array of netlink attributes) is populated in result. Following this, all other fields in `dpif_upcall` are populated:

```c
    type = (genl->cmd == OVS_PACKET_CMD_MISS ? DPIF_UC_MISS
            : genl->cmd == OVS_PACKET_CMD_ACTION ? DPIF_UC_ACTION
            : -1);

    /* (Re)set ALL fields of '*upcall' on successful return. */
    upcall->type = type;
    upcall->pmd_thread_id = PMD_ID_NULL;
    upcall->key = CONST_CAST(struct nlattr *,
                             nl_attr_get(a[OVS_PACKET_ATTR_KEY]));
    upcall->key_len = nl_attr_get_size(a[OVS_PACKET_ATTR_KEY]);
```

It is also at this place that the key gotten from fast path is converted to `ufid` by running it through a hash

```c
    dpif_flow_hash(&dpif->dpif, upcall->key, upcall->key_len, &upcall->ufid);
```

packet attribute is also set here:

```c
    dp_packet_set_data(&upcall->packet,
                    (char *)dp_packet_data(&upcall->packet) + sizeof(struct nlattr));
    dp_packet_set_size(&upcall->packet, nl_attr_get_size(a[OVS_PACKET_ATTR_PACKET]));
```

## Some Utility Functions

* `ofp_packet_to_string` takes a packet (from `dp_packet`) and returns a string that has printable version of the packet. The caller has to free the data returned. 
* `odp_flow_format` takes a key coming from the data path and converts to a string. It can also replace openflow port numbers with names if a corresponding hash is provided.


# Some Nuggets from OpenVSwitch Mailing Lists

* This [response](https://mail.openvswitch.org/pipermail/ovs-discuss/2013-August/030744.html) from Joe contains some information about *resubmit* and *recirculate* actions - where it is done.
* One [response](https://mail.openvswitch.org/pipermail/ovs-discuss/2013-August/030855.html) from Reid Price on how the *internal* interface for a bridge is used.

