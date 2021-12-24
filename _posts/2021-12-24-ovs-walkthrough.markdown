---
layout: post
title: "OVS Code Walkthrough Notes"
date:  2021-12-24 19:02:26 +0530
categories: ovs networking
---

## Some Data Structures

### udpif

`struct udpif` is basically a upcall handler context that has information about the set of `handlers` that handle the upcalls, set of `revalidators` that handle the flow updation in the fast path, `dpif` the datapath handle, `dump_seq` for dumping all the flows and `reval_seq` for controlling the revalidation, `ukeys` that contain the hash of flow keys which are installed in the fast path and other flow related statistics such as `flow_limit`, `n_flows`, `max_n_flows` etc.

### ofpbuf

`struct ofpbuf` is a descriptor of a buffer which is gotten through some means (from heap or stack etc.). It contains various pointers into the buffer such as `base`, `data`, `header`, `msg` etc and its capacity in `size, allocated`. It can be made to point to some memory area by calling `ofpbuf_use_stub`, for example.

### dpif_upcall

`struct dpif_upcall` contains information about a packet sent from the data path (be it kernel or DPDK). It contains the `packet` itself, `key`, a set of netlink attributes that describe various packet metadata (like input port or tunnel etc.), `ufid` the unique identifier for a flow (in case of revalidation) etc.

### upcall

`struct upcall` contains the context needed for installing a flow in the data path. It contains `flow` which represents the packet's needed parameters, `in_port` from which the packet came in, `xout` result of translating actions, `odp_actions` a set of data path actions, `wc` the wildcard masks that are accumulated by walking through the ofproto tables, `dump_seq` and `reval_seq` the values of the global sequence number cached for later comparison etc.

### flow

`struct flow` is the one that contains all the needed packet data and meta-data required to match a flow. `flow` contains five sections of data. Metadata, L2 fields, L3 fields, L4 fields and L7 fields. OVS does a staged lookup, so it does access them in this order. Metadata principally consists of `regs` that hold various pieces of information during staged lookup, `skb_priority`, `pkt_mark`, `dp_hash`, `in_port`, `recirc_id`, `actset_output` the output port to which the packets needs to be sent out and other conntrack related information. Other layer fields are straightforward, L7 fields need some explanation.

### flow_wildcards

This is just a `struct flow` in disguise

### miniflow

A `miniflow` is a compressed version of a `flow` structure. It consists of two parts:

```c
struct miniflow {
    struct flowmap map;
    /* Followed by:
     *     uint64_t values[n];
     * where 'n' is miniflow_n_values(miniflow). */
};
```

The `map` is explicitly part of the `miniflow` structure and contains two `uint64_t`s where each bit tells us which 64-bit segment in the `flow` structure is non-zero. The second part consists of those 64-bit segments from `flow` which are non-zero (or to be considered). Refer to the links below that gives a pictorial representation of the same.

### dpif_backer

This structure acts as a bridge between user space and fast path. It holds `dpif` the handle for kernel fast path global structure, `udpif` the handle for upcall global context, `odp_to_ofport_map` the hash map that maps fast path port to openflow port etc.

### ofproto

`ofproto` is the openflow implementation of a switch. It embodies `oftable`s which contain rules that match on some parameters and spell out an action. 

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

Then we extract packet fields into the `flow` (earlier we translated netlink attributes to `flow` with `odp_flow_key_to_flow`) and finally process the upcall:

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

### upcall_receive

After `dpif_recv` populates `dupcall` (`dpif_upcall`) with needed information, this function converts the same to `upcall` for further processing. In addition to initializing upcall fields and populating some with the passed parameters, one of the other things that it does is find the `ofproto` and `ofp_in_port` for the given context. Without that, the upcall handling cannot proceed

```c
    error = xlate_lookup(backer, flow, &upcall->ofproto, &upcall->ipfix,
                         &upcall->sflow, NULL, &upcall->in_port);
```

### xlate_lookup

This function tries to find them by calling `xlate_lookup_ofproto_`

```c
    ofproto = xlate_lookup_ofproto_(backer, flow, ofp_in_port, &xport);
```

As mentioned above, the `xport`, `xbridge`, `xbundle` data structures are used to map user space to fast path and vice versa. If we look into `xlate_lookup_ofproto_` function:

```c
    xport = xport_lookup(xcfg, tnl_port_should_receive(flow)
                         ? tnl_port_receive(flow)
                         : odp_port_to_ofport(backer, flow->in_port.odp_port));
    *xportp = xport;
    if (ofp_in_port) {
        *ofp_in_port = xport->ofp_port;
    }
    return xport->xbridge->ofproto;
```

it does a `xport_lookup` from `xcfg` which is basically of type `struct xlate_cfg` that has hash maps of all the xports, xbridges and xbundles. In case, the packet has come in a tunnel, `tnl_port_should_receive` will return true and we will get the openflow port of tunnel by calling `tnl_port_receive`, else we simply call `odp_port_to_ofport`.

```c
struct ofport_dpif *
odp_port_to_ofport(const struct dpif_backer *backer, odp_port_t odp_port)
{
    HMAP_FOR_EACH_IN_BUCKET (port, odp_port_node, hash_odp_port(odp_port),
                             &backer->odp_to_ofport_map) {
        if (port->odp_port == odp_port) {
            return port;
        }
    }
    return NULL;
}
```

The `odp_port_to_ofport` function will look at the `odp_to_ofport_map` hash in `backer` and retrieve the corresponding ofport.

### flow_extract

This function takes `dp_packet` which contains the packet per se and the packet metadata and converts it into a flow. `packet->md` is populated from `flow` itself which is populated from `odp_flow_key_to_flow` - a function that populates stuff such as `recirc_id`, `dp_hash`, `pkt_mark`, `skb_priority` and other conntrack related states. `flow_extract` converts a packet into a `flow` via a `miniflow`:

```c
    struct {
        struct miniflow mf;
        uint64_t buf[FLOW_U64S];
    } m;

    miniflow_extract(packet, &m.mf);
    miniflow_expand(&m.mf, flow);
```

It extracts the packet contents into a `miniflow` and then expands it to a `flow`

### miniflow_extract

To begin with, the passed miniflow is taken into a `mf_ctx` - the `data` in `mf` points to the values that start after the `map` in `miniflow`:

```c
    struct mf_ctx mf = { FLOWMAP_EMPTY_INITIALIZER, values,
                         values + FLOW_U64S };
```

And here a long journey of going through different attributes in `packet` begins and then the corresponding bits are set in the `miniflow`. Let's first look at some miniflow utility functions.

One of the macros looks like this:

```c
#define miniflow_push_words(MF, FIELD, VALUEP, N_WORDS)                 \
    miniflow_push_words_(MF, offsetof(struct flow, FIELD), VALUEP, N_WORDS)
```

It takes the `mf_ctx` in `MF`, the `FIELD` in the `flow` structure that is being pushed (the set of values that follow after the map - are appended), the pointer to the data `VALUEP` and number of words. Let's take a simple example from `miniflow_extract`:

```c
    if (flow_tnl_dst_is_set(&md->tunnel)) {
        miniflow_push_words(mf, tunnel, &md->tunnel,
                            offsetof(struct flow_tnl, metadata) /
                            sizeof(uint64_t));
```

If tunnel is set, then this snippet pushes everything in flow_tnl, except metadata, in units of 8 bytes. Also, `md->tunnel` is where the data is available and `tunnel` the field in `flow` structure where this data is supposed to be copied. Since this is a `miniflow`, it will set the corresponding bitmap in the `map` and copies the data to `values`:

```c
#define miniflow_push_words_(MF, OFS, VALUEP, N_WORDS)          \
{                                                               \
    MINIFLOW_ASSERT((OFS) % 8 == 0);                            \
    miniflow_set_maps(MF, (OFS) / 8, (N_WORDS));                \
    memcpy(MF.data, (VALUEP), (N_WORDS) * sizeof *MF.data);     \
    MF.data += (N_WORDS);                                       \
}
```

The `mf_ctx` which is initially populated with passed flow map and its data, is used by these functions to fill it. The bitmap is set through `flowmap_set` which is called from `miniflow_set_maps`:

```c
flowmap_set(struct flowmap *fm, size_t idx, unsigned int n_bits)
{
    map_t n_bits_mask = (MAP_1 << n_bits) - 1;
    size_t unit = idx / MAP_T_BITS;

    idx %= MAP_T_BITS;

    fm->bits[unit] |= n_bits_mask << idx;
    if (unit + 1 < FLOWMAP_UNITS && idx + n_bits > MAP_T_BITS) {
        fm->bits[unit + 1] |= n_bits_mask >> (MAP_T_BITS - idx);
    }
}
```

The if clause catches the situation where the offset in the bitmap, together with number of bits to be set, crosses the unit boundary. In such case, the remaining bits are set in the next unit (unit 1).  Lets look at one other such macro:

```c
#define miniflow_push_uint32_(MF, OFS, VALUE)   \
    {                                           \
    if ((OFS) % 8 == 0) {                       \
        miniflow_set_map(MF, OFS / 8);          \
        *(uint32_t *)MF.data = VALUE;           \
    } else if ((OFS) % 8 == 4) {                \
        miniflow_assert_in_map(MF, OFS / 8);    \
        *((uint32_t *)MF.data + 1) = VALUE;     \
        MF.data++;                              \
    }                                           \
}
```
When a `uint32` is to be set, it can either fall at the 8 byte boundary or it can fall at the 4 byte boundary (that is how `flow` is designed). If it is the former case, then `data` at current position is set with value. Since `data` can hold 4 more bytes, it is not incremented here. If the given value, in `flow`, falls at the 4 byte boundary, then the remaining 4 bytes at `data` is written to and the `data` pointer is incremented. Lets look at another one:

```c
#define miniflow_push_uint16_(MF, OFS, VALUE)   \
{                                               \
    MINIFLOW_ASSERT(MF.data < MF.end);          \
                                                \
    if ((OFS) % 8 == 0) {                       \
        miniflow_set_map(MF, OFS / 8);          \
        *(uint16_t *)MF.data = VALUE;           \
    } else if ((OFS) % 8 == 2) {                \
        miniflow_assert_in_map(MF, OFS / 8);    \
        *((uint16_t *)MF.data + 1) = VALUE;     \
    } else if ((OFS) % 8 == 4) {                \
        miniflow_assert_in_map(MF, OFS / 8);    \
        *((uint16_t *)MF.data + 2) = VALUE;     \
    } else if ((OFS) % 8 == 6) {                \
        miniflow_assert_in_map(MF, OFS / 8);    \
        *((uint16_t *)MF.data + 3) = VALUE;     \
        MF.data++;                              \
    }                                           \
}
```

Going by the above logic for `miniflow_push_uint32` this should be pretty straightforward to follow. Only in the case where the 2 bytes that are to be written are in the last two bytes of `data`, it is written and `data` is incremented.

Pushing metadata from the `dp_packet` to `miniflow` is achieved through the above macros. When it comes to adding packet data to the `miniflow`, the packet must first be parsed. Let's look at how L2 fields are copied to `miniflow`:

```c
        ASSERT_SEQUENTIAL(dl_dst, dl_src);
        miniflow_push_macs(mf, dl_dst, data);
        /* dl_type, vlan_tci. */
        vlan_tci = parse_vlan(&data, &size);
        dl_type = parse_ethertype(&data, &size);
        miniflow_push_be16(mf, dl_type, dl_type);
        miniflow_push_be16(mf, vlan_tci, vlan_tci);
```

The first macro asserts that `dl_dst` and `dl_src` are next to each other in `flow` because the next line `miniflow_push_macs` pushes both of them in one shot. `parse_vlan` returns the vlan id, if any, and `parse_ethertype` returns the eth type (after going past the vlan header). They are pushed to `miniflow`. I wonder why a 0 `vlan_tci` is pushed? Similarly, all the L3, L4 header fields are parsed later. For L3, IPv4 and IPv6 are parsed, for IPv6 extension headers are parsed, some classic L4 protocols are parsed. L7 is left, it is not touched here. Finally, the `map` in `mf_ctx` is put back in the original `miniflow` structure that is passed:

```c
 out:
    dst->map = mf.map;
```

### miniflow_expand

`miniflow_expand` is a simple function, which calls another one:

```c
miniflow_expand(const struct miniflow *src, struct flow *dst)
{
    memset(dst, 0, sizeof *dst);
    flow_union_with_miniflow(dst, src);
}
```

`flow_union_with_miniflow` calls the below function which walks through the miniflow and fills the flow. This filling is simple because the `dst` is converted to `uint64_t` and then fills the `dst` with the corresponding value from the `miniflow` if the corresponding bit is set in the `map`

```c
flow_union_with_miniflow_subset(struct flow *dst, const struct miniflow *src,
                                struct flowmap subset)
{
    uint64_t *dst_u64 = (uint64_t *) dst;
    const uint64_t *p = miniflow_get_values(src);
    map_t map;

    FLOWMAP_FOR_EACH_MAP (map, subset) {
        size_t idx;

        MAP_FOR_EACH_INDEX(idx, map) {
            dst_u64[idx] |= *p++;
        }
        dst_u64 += MAP_T_BITS;
    }
}
```

### process_upcall

Once all the needed information is transformed/ported to `upcall`, `process_upcall` is called to de-mux further processing based on the type of the upcall. The `classify_upcall` function basically converts a `dpif_upcall_type` to a `upcall_type`. `userdata` is used only if the upcall type is action. We are interested in walking through the miss upcall path, so we will be interested in what `upcall_xlate` does:

```c
static int
process_upcall(struct udpif *udpif, struct upcall *upcall,
               struct ofpbuf *odp_actions, struct flow_wildcards *wc)
{
    const struct nlattr *userdata = upcall->userdata;
    const struct dp_packet *packet = upcall->packet;
    const struct flow *flow = upcall->flow;

    switch (classify_upcall(upcall->type, userdata)) {
    case MISS_UPCALL:
        upcall_xlate(udpif, upcall, odp_actions, wc);
        return 0;
```

### upcall_xlate


## Some Utility Functions

* `ofp_packet_to_string` takes a packet (from `dp_packet`) and returns a string that has printable version of the packet. The caller has to free the data returned. 
* `odp_flow_format` takes a key coming from the data path and converts to a string. It can also replace openflow port numbers with names if a corresponding hash is provided.


# Some Links that Contain Useful Information

* This [response](https://mail.openvswitch.org/pipermail/ovs-discuss/2013-August/030744.html) from Joe contains some information about *resubmit* and *recirculate* actions - where it is done.
* One [response](https://mail.openvswitch.org/pipermail/ovs-discuss/2013-August/030855.html) from Reid Price on how the *internal* interface for a bridge is used.
* This Intel article [about miniflows](https://software.intel.com/content/www/us/en/develop/articles/open-vswitch-subtable-show-tool.html) is informative, with pictures!

