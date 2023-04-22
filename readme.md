
========== Instructions for installing the Linux tool chain for building the SDK ==========

1. Download the tool chain from link below:
   
   https://meta.box.lenovo.com/v/link/view/af115a5f96ab4601b204b6eb994f352c

   with password: telink123

   The current version of the tool is in folder "Google Ref RCU For ATV/Linux Tool Chain"

2. Extract the downloaded tool chain packege.
3. cd into the extracted folder.
4. Follow the instructions in the readme file to install the tool chain.


========== Instructions for adding library files into the SDK ==========

1. Download the library files from link below:

   https://meta.box.lenovo.com/v/link/view/af115a5f96ab4601b204b6eb994f352c

   with password: telink123

   The current version of the library files are in folder "Google Ref RCU For ATV/Pro_Lib"

2. EXtract the tgz file and copy the .a files into the folder "proj_lib" under the SDK directory "Google_ATV_RCU_Reference_SDK".


========== Instructions for building the SDK ==========

1. cd into the "8278_ble_remote" directory under the SDK directory "Google_ATV_RCU_Reference_SDK".
2. Type "make" to build the SDK or "make clean" to clean the project. 


========== Instructions for installing the burning tool via 3 (power, SWS, ground) wires ==========

1. Download the SWS 3-wire burning tool from the link below

   https://meta.box.lenovo.com/v/link/view/af115a5f96ab4601b204b6eb994f352c

   with password: telink123

   The current version of the tool is in folder "Google Ref RCU For ATV/Linux_tcdb"

2  Extact the downloaded tgz file into a a directory and cd into the directory.

3. Run ./install_tcdb.sh to install the tool. Or

4. Follow the installation instructions in the document "Telink Linux tcdb user guide.pdf" to install the tool.


========== Instructions for burning a bin file into RCU via 3 (power, SWS, ground) wires ==========

1. Follow the instructions in the document "Telink Linux tcdb user guide.pdf" to flash a bin file.



========== Instructions for updateing the new bin file into RCU via OTA process ==========

1. To be added
