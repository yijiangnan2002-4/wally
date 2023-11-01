************************************************
*********** nvdm post build ********************
************************************************

nvdm_post_build is to support generating nvdm image based on NV key xmls.

	linux/nvdm_post_build     - linux version
	windows/nvdm_post_build.exe - windows version
	mac/nvdm_post_build         - MAC version

[Usage]
-o [output folder] The folder where the nvdm.bin or nvdm_ou.bin will be generated.
-s [source NV xmls] NV xmls which defines NV keys and values. Using ',' to seperate XML filenames.
-size [size_in_Kbytes] size of output nvdm.bin or nvdm_ou.bin
-mode [mode] 1: generate nvdm_ou.bin with NV keys not in in domain "MP_DATA", "USER_DATA" and "BT_CON_DATA"
             2: generate nvdm.bin with NV keys in domain "MP_DATA", "USER_DATA" and "BT_CON_DATA"
             3: generate nvdm.bin including all NV keys
-allow_duplicated_nv_id [true/false] allow duplicated NV ID or not. 
                                     1. default is 'true'
                                     2. nvdm.bin will not be generated if duplicated NV ID existed when allow_duplicated_nv_id set to 'false'
 
Example:

A. nvdm_post_build -f nv2bin -o "2833/" -s "nvkey.xml,nvkey_chip.xml" -size 96 -mode 3

	- generated 2833/nvdm.bin sized to 96K with all NV keys

B. nvdm_post_build -f nv2bin -o "2833/" -s "nvkey.xml" -size 64 -mode 1

	- generated 2833/nvdm_ou.bin sized to 64K with NV keys not in in domain "MP_DATA", "USER_DATA" and "BT_CON_DATA"

************************************************
*********** media files to binary **************
************************************************

mediaFiles2bin to support generating filesystem image based on FileManagement.xml.

	linux/mediaFiles2bin        - linux version
	windows/mediaFiles2bin.exe  - windows version
	mac/mediaFiles2bin          - MAC version

[Usage]
mediaFile2bin.exe -f mediaFiles2bin <xml_filename> <media_folder> <output_bin_size_KBytes> <output_bin_filename>
The following command will parse XML information to store image file. Also the "size" parameter is used to specify the size of the image sector size.

e.g. mediaFile2bin.exe -f mediaFiles2bin FileManagement.xml FileSystem 512 filesystem.bin

