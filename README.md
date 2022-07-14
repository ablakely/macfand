# macfand 
Fan control daemon for Apple Computers running Linux through the applesmc kernel module.

## libconfig
macfand requires [libconfig](http://hyperrealm.github.io/libconfig/) which is available on many distrobutions.

### Debian / Ubuntu / Mint / Elementary OS / MX / Pop!_OS / Zorin

    sudo apt-get install libconfig-dev

### Arch / Manjaro / EndeavourOS 

    sudo pacman -S libconfig

### Fedora / CentOS / Rocky

    sudo yum install libconfig

### Gentoo

    sudo emerge dev-libs/libconfig

### openSUSE

    sudo zypper install libconfig-devel

### Alpine

    sudo apk add libconfig


## Installing

    make
    sudo make install
    
    # Edit /etc/macfand.conf
    
    sudo systemctl enable --now macfand

macfand is available in the [Arch User Repository](https://aur.archlinux.org/packages/macfand-git)


## Compatibility

macfand uses profiles configurations (located in the machines directory) to provide compatiblity with many machines as well as the ability to tune the performance of the fans for lower decebals.

| Device Identifer  | Confirmed | Untested  | Not Supported |
|-------------------|-----------|-----------|---------------|
| macpro1,1         | ✅        |           |               |
| macpro2,1         | ✅        |           |               |
| macpro3,1         |           | ✅        |               |
| macpro4,1         |           | ✅        |               |
| macpro5,1         |           | ✅        |               |
| macpro6,1         |           | ✅        |               |
| imac4,1           |           | ✅        |               |
| imac5,1           |           | ✅        |               |
| imac5,2           |           | ✅        |               |
| imac6,1           |           | ✅        |               |
| imac7,1           |           | ✅        |               |
| imac8,1           |           | ✅        |               |
| imac9,1           |           | ✅        |               |
| imac10,1          |           | ✅        |               |
| imac11,1          |           | ✅        |               |
| imac11,2          |           | ✅        |               |
| imac11,3          |           | ✅        |               |
| imac12,1          |           | ✅        |               |
| imac12,2          |           | ✅        |               |
| imac13,1          |           | ✅        |               |
| imac13,2          |           | ✅        |               |
| imac14,1          |           | ✅        |               |
| imac14,2          |           | ✅        |               |
| imac14,3          |           | ✅        |               |
| imac14,4          |           | ✅        |               |
| imac15,1          |           | ✅        |               |
| imac16,1          |           | ✅        |               |
| imac17,1          |           | ✅        |               |
| imac18,1          |           | ✅        |               |
| imac18,2          |           | ✅        |               |
| imac18,3          |           | ✅        |               |
| imacpro1,1        |           | ✅        |               |
| macbook1,1        |           | ✅        |               |
| macbook2,1        |           | ✅        |               |
| macbook3,1        |           | ✅        |               |
| macbook4,1        |           | ✅        |               |
| macbook5,1        |           | ✅        |               |
| macbook5,2        |           | ✅        |               |
| macbook6,1        |           | ✅        |               |
| macbook7,1        | ✅        |           |               |
| macbook8,1        |           | ✅        |               |
| macbook9,1        |           | ✅        |               |
| macbook10,1       |           | ✅        |               |
| macbookair1,1     |           | ✅        |               |
| macbookair2,1     |           | ✅        |               |
| macbookair3,1     |           | ✅        |               |
| macbookair3,2     |           | ✅        |               |
| macbookair4,1     |           | ✅        |               |
| macbookair4,2     |           | ✅        |               |
| macbookair5,1     |           | ✅        |               |
| macbookair5,2     |           | ✅        |               |
| macbookair6,1     |           | ✅        |               |
| macbookair6,2     |           | ✅        |               |
| macbookair7,1     |           | ✅        |               |
| macbookair7,2     |           | ✅        |               |
| macbookpro1,1     |           | ✅        |               |
| macbookpro1,2     |           | ✅        |               |
| macbookpro2,1     |           | ✅        |               |
| macbookpro2,2     |           | ✅        |               |
| macbookpro3,1     |           | ✅        |               |
| macbookpro4,1     |           | ✅        |               |
| macbookpro5,1     |           | ✅        |               |
| macbookpro5,2     |           | ✅        |               |
| macbookpro5,3     |           | ✅        |               |
| macbookpro5,4     |           | ✅        |               |
| macbookpro5,5     |           | ✅        |               |
| macbookpro6,1     |           | ✅        |               |
| macbookpro6,2     |           | ✅        |               |
| macbookpro7,1     |           | ✅        |               |
| macbookpro8,1     |           | ✅        |               |
| macbookpro8,2     |           | ✅        |               |
| macbookpro8,3     |           | ✅        |               |
| macbookpro9,1     |           | ✅        |               |
| macbookpro9,2     |           | ✅        |               |
| macbookpro10,1    |           | ✅        |               |
| macbookpro10,2    |           | ✅        |               |
| macbookpro11,1    |           | ✅        |               |
| macbookpro11,3    |           | ✅        |               |
| macbookpro11,4    |           | ✅        |               |
| macbookpro11,5    |           | ✅        |               |
| macbookpro12,1    |           | ✅        |               |
| macbookpro13,1    |           | ✅        |               |
| macbookpro13,2    |           | ✅        |               |
| macbookpro13,3    |           | ✅        |               |
| macbookpro14,1    |           | ✅        |               |
| macbookpro14,2    |           | ✅        |               |
| macbookpro14,3    |           | ✅        |               |
| macbookpro15,1    |           | ✅        |               |
| macbookpro15,2    |           | ✅        |               |
| macmini1,1        |           | ✅        |               |
| macmini2,1        |           | ✅        |               |
| macmini3,1        |           | ✅        |               |
| macmini4,1        |           | ✅        |               |
| macmini5,1        |           | ✅        |               |
| macmini5,2        |           | ✅        |               |
| macmini5,3        |           | ✅        |               |
| macmini6,1        |           | ✅        |               |
| macmini6,2        |           | ✅        |               |
| macmini7,1        |           | ✅        |               |
| xserve1,1         |           | ✅        |               |
| xserve2,1         |           | ✅        |               |
| xserve3,1         |           | ✅        |               |

---
See the manual page `macfand(1)` for more information.

Copyright &copy; 2022 Aaron Blakely


Support for this software is available on IRC `irc.ephasic.org in #macfand`