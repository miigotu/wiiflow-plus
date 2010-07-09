::----------------------------------------------::
::-------------------Usage----------------------::
::This file is designed to be used with sendelf ::
:: and Gecko Reader								::
::Use:											::
::1. Install sendelf and configure it to use	::
::either usbGecko or tcp and add the path of	::
::sendelf to your PATH environment variable.	::
::2. Get Gecko Reader and extract it somewhere	::
:: out of the way and also set your PATH to 	::
::include the path to geckoreader.exe			::
::Afterwards you can simply type "make && run"	::
::or "make ios222 && run" to make the compiled	::
::DOL file be loaded to the wii for testing as	::
::soon as compiling is complete.              	::
::---------------------------------------------::
@echo off
echo Sending...
wiiflow.dol > sendelf
sleep 3
GeckoReader