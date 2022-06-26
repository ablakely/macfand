# macfand 
Fan control daemon based on the abandoned macfanctld

## Installing
    make
    sudo make install
    
    # Edit /etc/macfand.conf
    
    sudo systemctl enable --now macfand

macfand is available in the [Arch User Repository](https://aur.archlinux.org/packages/macfand-git)


## Compatibility

| Device Identifer  | Confirmed | Likely    | Unknown | Planned |
|-------------------|-----------|-----------|---------|---------|
| macpro1,1         |✅          |           |         |         |
| macpro2,1         |✅          |           |         |         |
| macpro3,1         |           |✅          |         |         |
| macpro4,1         |           |✅          |         |         |
| macpro5,1         |           |✅          |         |         |
| macbookpro11,5    |           |           |         |✅        |

macbooks are currently untested but should function, will be testing with a mid-2015 pro soon.

---
See the manual page `macfand(1)` for more information.

Copyright &copy; 2022 Aaron Blakely


Support for this software is available on IRC `irc.ephasic.org in #macfand`