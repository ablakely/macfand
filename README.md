# macprofanctld 
Fan control daemon based on the abandoned macfanctld

## Installing
    make
    sudo make install
    
    # Edit /etc/macprofanctld.conf
    
    sudo systemctl enable --now macprofanctld


## Compatibility

# macprofanctld 
Fan control daemon based on the abandoned macfanctld

## Installing
    make
    sudo make install
    
    # Edit /etc/macprofanctld.conf
    
    sudo systemctl enable --now macprofanctld


## Compatibility

| Device Identifer  | Confirmed | Likely | Unknown | Planned|
| ---               |:---      :| :---  :|:---    :|: ---   |
| macpro1,1         |✅          |        |         |        |
| macpro2,1         |✅          |        |         |        |
| macpro3,1         |           |✅       |         |        |
| macpro4,1         |           |✅       |         |        |
| macpro5,1         |           |✅       |         |        |

macbooks are currently not supported by this daemon, will be testing with a mid-2015 pro soon.

---
See the manual page `macprofanctld(1)` for more information.

Copyright &copy; 2022 Aaron Blakely
