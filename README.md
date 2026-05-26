# bms
bms stands for Battery Management System, it's a simple linux utility designed to extend the battery usage from a notebook or a system which has battery.
For now it's only will cap the processor (INTEL only) TDP and clock, in the future it will analyze and permit or block "usage" from the CPU, GPU if it has, and even screen duration, besides it will be able to estimate with these new changes the remaining time of the battery.



## Usage

For now bms has few commands and options, all of listed just by running the code:

```
./sudo bms
Current Battery Level: 100%  
Usage: ./bms [nsave|msave|hsave|auto]                                                                                                                                      
Options:
     nsave - No power saving (Default limits)
     msave - Medium power saving (50% limit)
     hsave - High power saving (20% limit)
     auto  - Automatically select mode based on battery level  
```


# Updates
More updates improving the the project code and architecture are to come. Any help is welcomed.
