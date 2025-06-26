.PHONY: all clean install
name=
all:
	@echo "---------------samba finish----------------"
install:
	@mkdir $(ROOTDIR)/samba/lib/ -p
	@cp -r ./config/** $(ROOTDIR)/samba/lib/
	@sed -i 's:/home/relech/share/:$(ROOTDIR):g' $(ROOTDIR)/samba/lib/smb.conf
	@sed -i 's:/home/relech/share/:$(ROOTDIR):g' $(ROOTDIR)/samba/lib/smbmedia.conf
clean:

