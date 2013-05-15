# Project: Edward

BIN  = ../edward

all : $(BIN)

install : all
	mkdir -p /usr/share/edward
	mkdir -p /usr/share/edward/Data
	cp -f ../Data/hui_resources.txt    /usr/share/edward/Data
	cp -f ../Data/icon.*               /usr/share/edward/Data
	cp -f ../Data/edward.jpg           /usr/share/edward/Data
	cp -rf ../Data/icons               /usr/share/edward/Data
	cp -f ../Data/keys.txt             /usr/share/edward/Data
	cp -f ../Data/license.txt          /usr/share/edward/Data
	cp -f ../Data/license_small.txt    /usr/share/edward/Data
	mkdir -p /usr/share/edward/Plugins
	cp -rf ../Plugins/*.kaba           /usr/share/edward/Plugins
	xdg-desktop-menu install ../Data/michisoft-edward.desktop
	xdg-mime install ../Data/michisoft-x-model.xml
	xdg-icon-resource install --context mimetypes --size  32 ../Data/icons/png/model_32.png  application-x-michisoft-x-model --noupdate
	xdg-icon-resource install --context mimetypes --size  48 ../Data/icons/png/model_48.png  application-x-michisoft-x-model --noupdate
	xdg-icon-resource install --context mimetypes --size  64 ../Data/icons/png/model_64.png  application-x-michisoft-x-model --noupdate
	xdg-icon-resource install --context mimetypes --size  128 ../Data/icons/png/model_128.png  application-x-michisoft-x-model
	xdg-mime install ../Data/michisoft-x-font.xml
	xdg-icon-resource install --context mimetypes --size  32 ../Data/icons/png/font_32.png  application-x-michisoft-x-font --noupdate
	xdg-icon-resource install --context mimetypes --size  48 ../Data/icons/png/font_48.png  application-x-michisoft-x-font --noupdate
	xdg-icon-resource install --context mimetypes --size  64 ../Data/icons/png/font_64.png  application-x-michisoft-x-font --noupdate
	xdg-icon-resource install --context mimetypes --size  128 ../Data/icons/png/font_128.png  application-x-michisoft-x-font
	xdg-mime install ../Data/michisoft-x-world.xml
	xdg-icon-resource install --context mimetypes --size  32 ../Data/icons/png/world_32.png  application-x-michisoft-x-world --noupdate
	xdg-icon-resource install --context mimetypes --size  48 ../Data/icons/png/world_48.png  application-x-michisoft-x-world --noupdate
	xdg-icon-resource install --context mimetypes --size  64 ../Data/icons/png/world_64.png  application-x-michisoft-x-world --noupdate
	xdg-icon-resource install --context mimetypes --size  128 ../Data/icons/png/world_128.png  application-x-michisoft-x-world
	xdg-mime install ../Data/michisoft-x-terrain.xml
	xdg-icon-resource install --context mimetypes --size  32 ../Data/icons/png/terrain_32.png  application-x-michisoft-x-terrain --noupdate
	xdg-icon-resource install --context mimetypes --size  48 ../Data/icons/png/terrain_48.png  application-x-michisoft-x-terrain --noupdate
	xdg-icon-resource install --context mimetypes --size  64 ../Data/icons/png/terrain_64.png  application-x-michisoft-x-terrain --noupdate
	xdg-icon-resource install --context mimetypes --size  128 ../Data/icons/png/terrain_128.png  application-x-michisoft-x-terrain
	cp $(BIN) /usr/bin
