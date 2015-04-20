# script to install necessary vm and kvm drivers

umount mnt.guest-disk
umount mnt.disk
mount disk.img mnt.disk
mount mnt.disk/root/disk.img mnt.guest-disk
cp /home/zhang/kvm/tzvisor/kvm/tzvmods/kvm-dev.ko mnt.disk/root/.
sync
cp /home/zhang/kvm/tzvisor/kvm/tzvmods/vm-dev.ko mnt.guest-disk/root/.
cp /home/zhang/kvm/tzvisor/kvm/tzvmods/user mnt.guest-disk/root/.
cp /home/zhang/kvm/tzvisor/kvm/tzvmods/mount.sh mnt.guest-disk/root/.
sync
cp /home/zhang/kvm/tzvisor/kvm/user/pal mnt.guest-disk/root/.
sync
umount mnt.guest-disk
umount mnt.disk
