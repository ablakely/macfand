# macfand 
Fan control daemon based on the abandoned macfanctld

## Installing
    make
    sudo make install
    
    # Edit /etc/macfand.conf
    
    sudo systemctl enable --now macfand


## Compatibility

macfand should work Mac Pros with system identifiers macpro1,1 through macpro5,1.
Support for macbooks is currently untested.


---
See the manual page macfand (1) for more information.
