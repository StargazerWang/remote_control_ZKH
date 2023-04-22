:: remove command prompt and this line
@echo off 



echo copy current .o file
::xcopy C:\__Sharp_vulture_rc\telink_vulture_ble_sdk\ble_sdk_multimode\8278_ble_remote\vendor\827x_ble_remote\custom_cmd_proc.o 
xcopy 8271_ble_remote\application\audio\gl_audio.o 

echo make .a
tc32-elf-ar rcs gl_audio.a gl_audio.o


echo move .a to proj_lib 
::xcopy custom_cmd_proc.a  tmp\
copy gl_audio.a  application\audio\libgl_audio.a


echo remove files
del gl_audio.o 
del gl_audio.a 
del application\audio\gl_audio.c 




pause