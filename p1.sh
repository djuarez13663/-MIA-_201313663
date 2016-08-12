mkdisk -size::100 -path::"/home/daniel/Escritorio/Discos/" \
-name::"d1.dsk"
mkdisk -size::60 -path::"/home/daniel/Escritorio/Discos/" -name::"d2.dsk"
mkdisk -size::60 -path::"/home/daniel/Escritorio/Discos/" -name::"d3.dsk"
mkdisk -size::60 -path::"/home/daniel/Escritorio/Discos/" -name::"d4.dsk"
fdisk -size::20 -path::"/home/daniel/Escritorio/Discos/d1.dsk" -name::"P1" +unit::M
fdisk -size::15 -path::"/home/daniel/Escritorio/Discos/d1.dsk" -name::"P2" +unit::M
fdisk -size::15 -path::"/home/daniel/Escritorio/Discos/d1.dsk" -name::"P3" +unit::M
fdisk -name::"Ext" -path::"/home/daniel/Escritorio/Discos/d1.dsk" -size::20 +unit::M +type::E
mount -name::"P1" -path::"/home/daniel/Escritorio/Discos/d1.dsk"
mount -name::"P2" -path::"/home/daniel/Escritorio/Discos/d1.dsk"
fdisk -name::"L1" -path::"/home/daniel/Escritorio/Discos/d1.dsk" -size::19 +type::L +unit::M
fdisk -name::"L2" -path::"/home/daniel/Escritorio/Discos/d1.dsk" -size::20 +type::L +unit::M
fdisk -name::"L3" -path::"/home/daniel/Escritorio/Discos/d1.dsk" -size::20 +type::L +unit::M
mount -name::"L1" -path::"/home/daniel/Escritorio/Discos/d1.dsk"
rmdisk -path::"/home/daniel/Escritorio/Discos/d4.dsk"
rep -id::vda1 -path::"/home/daniel/Escritorio/Discos/Reportes/disco.png" -name::disk
rep -id::vda1 -path::"/home/daniel/Escritorio/Discos/Reportes/disco1.png" -name::mbr
fdisk +delete::full -name::"P3" -path::"/home/daniel/Escritorio/Discos/d1.dsk"
fdisk +delete::fast -name::"L1" -path::"/home/daniel/Escritorio/Discos/d1.dsk"
fdisk +add::-10 +unit::M -name::"L1" -path::"/home/daniel/Escritorio/Discos/d1.dsk"
fdisk -name::"L15" -path::"/home/daniel/Escritorio/Discos/d1.dsk" -size::3 +type::L +unit::M
fdisk -name::"L10" -path::"/home/daniel/Escritorio/Discos/d1.dsk" -size::2 +type::L +unit::M
fdisk -name::"L5" -path::"/home/daniel/Escritorio/Discos/d1.dsk" -size::2 +type::L +unit::M
fdisk -size::10 -path::"/home/daniel/Escritorio/Discos/d1.dsk" -name::"P5" +unit::M
fdisk +add::-8 +unit::M -path::"/home/daniel/Escritorio/Discos/d1.dsk" \
-name::"P5"
rep -id::vda1 -path::"/home/daniel/Escritorio/Discos/Reportes/disco2.png" -name::mbr
rep -id::vda1 -path::"/home/daniel/Escritorio/Discos/Reportes/disco3.png" -name::disk
fdisk +delete::fast -name::"P1" -path::"/home/daniel/Escritorio/Discos/d1.dsk"
mount
umount -id1::vda1 -id2::vda2 -id3::vda3
mount