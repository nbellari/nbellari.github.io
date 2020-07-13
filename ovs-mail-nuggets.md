# Code Walkthrough Notes

`struct udpif` is basically a upcall handler context that has information about the set of `handlers` that handle the upcalls, set of `revalidators` that handle the flow updation in the fast path, `dpif` the datapath handle, `dump_seq` for dumping all the flows and `reval_seq` for controlling the revalidation, `ukeys` that contain the hash of flow keys which are installed in the fast path and other flow related statistics such as `flow_limit`, `n_flows`, `max_n_flows` etc.

# Some Nuggets from OpenVSwitch Mailing Lists

This [response](https://mail.openvswitch.org/pipermail/ovs-discuss/2013-August/030744.html) from Joe contains some information about *resubmit* and *recirculate* actions - where it is done.

One [response](https://mail.openvswitch.org/pipermail/ovs-discuss/2013-August/030855.html) from Reid Price on how the *internal* interface for a bridge is used.

