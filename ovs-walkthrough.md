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

`dpif_recv` fills the `dupcall` with necessary information by storing any packet data in `recv_buf`. 

## Some Utility Functions

`ofp_packet_to_string` takes a packet (from `dp_packet`) and returns a string that has printable version of the packet. The caller has to free the data returned. `odp_flow_format` takes a key coming from the data path and converts to a string. It can also replace openflow port numbers with names if a corresponding hash is provided.


# Some Nuggets from OpenVSwitch Mailing Lists

This [response](https://mail.openvswitch.org/pipermail/ovs-discuss/2013-August/030744.html) from Joe contains some information about *resubmit* and *recirculate* actions - where it is done.

One [response](https://mail.openvswitch.org/pipermail/ovs-discuss/2013-August/030855.html) from Reid Price on how the *internal* interface for a bridge is used.

