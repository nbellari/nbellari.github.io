# OVS Metering Support

This page details the data structures/fields/functions that realize the metering support in OVS. Depending on the version you are looking at, there can be some discrepancies

## Some Terminology

* OFPRR - Open Flow Protocol Remove Reason (for a flow removal)
* OFPT - Open Flow Protocol Types (?)
* OFPMP - Open Flow Protocol MultiPart message types
* OFPIT - Open Flow Protocol Instruction Type
* OFPIT13 - Open Flow Protocol Instruction Type, introduced in openflow 1.3
* OFPMC - Open Flow Protocol Meter Command
* OFPMF - Open Flow Protocol Meter Flags
* OFPMBT - Open Flow Protocol Meter Band Type


## Data Structures

The meter instruction is conveyed through the following TLV structure:

```c
 95 struct ofp13_instruction_meter {
 96     ovs_be16 type;              /* OFPIT13_METER */
 97     ovs_be16 len;               /* Length is 8. */
 98     ovs_be32 meter_id;          /* Meter instance. */
 99 };

```

The meter action structure has two meter ids - one specified by the protocol and the other allocated by the provider:

```c
 635 struct ofpact_meter {
 636     OFPACT_PADDED_MEMBERS(
 637         struct ofpact ofpact;
 638         uint32_t meter_id;
 639         uint32_t provider_meter_id;
 640     );
 641 };
```

`ofp13_meter_band_header` has the rate and burst size which applies to a packet when it is classified under this band:

```c
116 /* Common header for all meter bands */
117 struct ofp13_meter_band_header {
118     ovs_be16 type;       /* One of OFPMBT_*. */
119     ovs_be16 len;        /* Length in bytes of this band. */
120     ovs_be32 rate;       /* Rate for this band. */
121     ovs_be32 burst_size; /* Size of bursts. */
122 };
123 OFP_ASSERT(sizeof(struct ofp13_meter_band_header) == 12);
```

Depending on what action is taken in the band, there are `ofp13_meter_band_drop` - which drops the packet when it falls under this band, `ofp13_meter_band_dscp_remark` - which rewrites the DSCP mark when it falls under the band and `ofp13_meter_band_experimenter` - which hints at custom actions. Note that all these structures can be type casted to `ofp13_meter_band_header` and all band types have the same length.

`ofputil_meter_request_type` - identifies the kind of meter request that has come:

```c
118 enum ofputil_meter_request_type {
119     OFPUTIL_METER_FEATURES,
120     OFPUTIL_METER_CONFIG,
121     OFPUTIL_METER_STATS
122 };
```

** Functions

`ofpacts_pull_openflow_instructions` pulls all the instructions from a given openflow message using the function `decode_openflow11_instructions` (inspite of the name). One interesting snippet in the function is like:

```c
8325         if (type == OVSINST_OFPIT13_METER && version >= OFP15_VERSION) {
8326             /* "meter" is an action, not an instruction, in OpenFlow 1.5. */
8327             return OFPERR_OFPBIC_UNKNOWN_INST;
8328         }
```

so, even though openflow 1.3 treats meter as an instruction, openflow 1.5 treats it as an action, as visible from the above snippet. Back in `ofpacts_pull_openflow_instructions`, an instruction is converted into `ofpact` as follows:

```c
8398     if (insts[OVSINST_OFPIT13_METER]) {
8399         const struct ofp13_instruction_meter *oim;
8400         struct ofpact_meter *om;
8401 
8402         oim = ALIGNED_CAST(const struct ofp13_instruction_meter *,
8403                            insts[OVSINST_OFPIT13_METER]);
8404 
8405         om = ofpact_put_METER(ofpacts);
8406         om->meter_id = ntohl(oim->meter_id);
8407         om->provider_meter_id = UINT32_MAX; /* No provider meter ID. */
8408     }
```

`ofputil_pull_bands` - extracts the meter bands for a meter. Similarly, `ofputil_put_bands` encodes the meter bands back into openflow message.


## Notes

* In the first open flow version, a flow mod message would carry the action directly, however in subsequent versions, the flow mod contains an instruction - which can modify an action set and much more. So, technically an instruction is superior to action
** Meter is not part of action, meter is a separate instruction as action is
* The latest openflow version is 1.5
* `OFPRAW_OFPT13_METER_MOD`, `OFPRAW_OFPST13_METER_REPLY`, `OFPRAW_OFPST13_METER_CONFIG_REQUEST, `OFPRAW_OFPST13_METER_CONFIG_REPLY` are some of the meter related messages
* A meter band is basically applies to a packet of a given flow when the rate of the packet is more than that measured in the band (the lowest among them). A meter band has a type associated with it that says whether it is drop or dscp-remark or some custom action (extensible). A given meter can have more than one band associated with it.
