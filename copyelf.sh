getbsssize(){
    bsssize=0
    if ! dd if="$elf" bs=1 skip=0 count=4 | od |
        grep -q '^0000000 042577 043114'
    then
        echo "ELF header not found in \`$elf'." >&2
        exit 1
    fi
    set -- $(dd if="$elf" bs=1 skip=28 count=4 | od -i)
    phoff=$2
    set -- $(dd if="$elf" bs=1 skip=42 count=2 | od -i)
    phentsize=$2
    set -- $(dd if="$elf" bs=1 skip=44 count=2 | od -i)
    phnum=$2
    while test $phnum -gt 0
    do
        phnum=$(($phnum-1))
        set -- $(($phoff+$phentsize*$phnum))
        set -- $(($1+16)) $(dd if="$elf" bs=1 skip=$1 count=4 | od -i)
        case "$3" in
        1)  ;;
        *)  continue;;
        esac
        set -- $(($1+4)) $(dd if="$elf" bs=1 skip=$1 count=4 | od -i)
        set -- "$3" $(dd if="$elf" bs=1 skip=$1 count=4 | od -i)
        bsssize=$(($bsssize+$3-$1))
    done
}

elf=$1
device=$2

getbsssize

{
    cat "$1"
    shift
    if test $bsssize -ge 512
    then
        dd if=/dev/zero count=$(($bsssize/512))
        bsssize=$(($bsssize%512))
    fi
    if test $bsssize -gt 0
    then
        dd if=/dev/zero bs=1 count=$bsssize
    fi
} | dd of="$device" conv=notrunc
