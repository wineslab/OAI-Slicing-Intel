# Slicing xApp Deployment 

This is a guide to deploy CCC model slicing xApp based on sdran-in-a-box.

The key idea is bridging the gap between slicing xAPP and OAI gNB based on the RIC in sdran. The interface between RIC and OAI gNB is the rrmPolicy.json 

## Customized RIC Deployment

This is installed on DU server.

### Docker registry 

Create docker registry point by 
```
docker run -d -p 5000:5000 --restart=always --name=registry registry:2
```


### Clone and Build docker Images for

- onos-e2-sm : https://github.com/intel-collab/networking.wireless.aether.onos-e2-sm
- onos-rsm : https://github.com/intel-collab/networking.wireless.aether.onos-rsm
- onos-topo : https://github.com/intel-collab/networking.wireless.aether.onos-topo
- onos-e2t : https://github.com/intel-collab/networking.wireless.aether.onos-e2t

For each repo above, run 
```
make images
```

Run `docker images` to check the builded docker images.

### RIC deployment

- Clone sdran-in-a-box from https://github.com/intel-collab/networking.wireless.aether.sdran-in-a-box.
- Run `make clean-all` if you have ever deploy old version RIC or k8s.
- Run `make OPT=ric VER=stable`


## Customized RAN-SIM Deployment

This is installed on DU server

- Clone ran-simulator from https://github.com/intel-collab/networking.wireless.aether.ran-simulator

- Change `rrmPolicyFilePath` in `pkg/servicemodel/ccc/msg2oai_json.go` according your OAI path.

- TODO: go module need to be carefully configured.

- run `./start_ransim.sh`. If it raises errors about go modules, go to the last step.

## OAI gNB with slicing functionality 

To build OAI gNB, you need first install `json-c` lib by `sudo apt install libjson-c-dev`.
Then, clone our repo
```
git clone https://github.com/wineslab/OAI-Slicing-Intel.git
cd OAI-Slicing-Intel
git checkout NR_UE_multi_pdusession
```
and follow the OAI official [guide](https://gitlab.eurecom.fr/oai/openairinterface5g/-/blob/develop/doc/NR_SA_Tutorial_COTS_UE.md?ref_type=heads) to build.


To start gNB, run the following commands 
```
cd /path/to/OAI-Slicing-Intel/cmake_targets/ran_build/build
sudo ./nr-softmodem -O ../../../targets/PROJECTS/GENERIC-NR-5GC/CONF/gnb.sa.band78.fr1.106PRB.usrpx310.sdcore.conf --sa --usrp-tx-thread-config 1
```


## RANSIM Slicing Control

This is running on CN server.

This is serving as slicing xApp, which communicates to RIC via the following curl command.

### Send Control Msg

Use curl to send control MSG to RIC
```
curl -X PUT -H "Content-Type: application/json" 10.21.64.12:31963/policytypes/ORAN_SliceSLATarget_1.0.0/policies/1 -d @policyRatio1.json
```
where `10.21.64.12` is the IP of DU at vlan.064.pr. If you prefer another vlan, change it accordingly. 


An example of policyRatio1.json:
```
{
    "scope": {
      "sliceId": {
        "sst": 1,
        "sd": "123456",
        "plmnId": {
          "mcc": "001",
          "mnc": "01"
        }
      }
    },

    "sliceSlaObjectives": {
      "maxNumberOfUes": 1,
      "maxNumberOfPduSessions": 2,
      "maxDlThptPerUe": 5,
      "guaDlThptPerSlice": 10,
      "maxDlThptPerSlice": 100
    }
}
```

To send control MSG about another slicing, create a `policyRatio2.json` like

```
{
    "scope": {
      "sliceId": {
        "sst": 1,
        "sd": "010203",
        "plmnId": {
          "mcc": "001",
          "mnc": "01"
        }
      }
    },

    "sliceSlaObjectives": {
      "maxNumberOfUes": 1,
      "maxNumberOfPduSessions": 2,
      "maxDlThptPerUe": 5,
      "guaDlThptPerSlice": 80,
      "maxDlThptPerSlice": 100
    }
}
```
