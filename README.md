<img align="left" width="80" height="80" src="https://raw.githubusercontent.com/mbruel/ex0days/master/icons/ex0days.png" alt="ex0days">

# ex0days v1.0

**Command line / GUI tool to automate extraction (or test) of double compressed 0days**<br/>
<br/>
![ex0days_v1.0](https://raw.githubusercontent.com/mbruel/ex0days/master/pics/ex0days_v1.0.png)
<br/>
ex0days relies on external programs to unpack: **unrar**, **unace** and **7z** for all other formats.<br/>
  - it will browse recursively the folders you provide.</li>
  - a 0day folder must **not** have subfolders.
  - it should contain **zip** files as **first compression** method
  - it generates a **csv log file** with the list of all broken 0days (in the logs folder where the app is)
  - you can just **run tests** (all temporary files will be deleted)
  - you can also **delete the source folders** automatically once extracted
  - **setting are saved** in a config file (ini file on Windows, ~/.config/ex0days/1.0.conf on Linux)
<br/>
A portable release containing all of them is available for win64: [ex0days_v1.0_win64.zip](https://github.com/mbruel/ex0days/releases/download/v1.0/ex0days_v1.0_win64.zip)<br/>


### How to build
ex0days is developped in **C++11 / Qt5** <br/>

#### Dependencies:
- build-essential (C++ compiler, libstdc++, make,...)
- qt5-default (Qt5 libraries and headers)
- qt5-qmake (to generate the moc files and create the Makefile)

#### Build:
- go to the src folder
- qmake
- make

Easy! it should have generate the executable **ex0days**</br>
you can copy it somewhere in your PATH so it will be accessible from anywhere

### How to use it in command line
<pre>
Syntax: ex0days (options)* (-i <src_folder>)+ -o <output_folder>
	-h or --help       : Help: display syntax
	-v or --version    : app version
	--debug            : display debug informations
	-i or --input      : input parent folder (containing 0days)
	-o or --output     : output folder (or temporary)
	-t or --test       : test only
	-d or --del        : delete sources once extracted
	--7z               : 7z full path
	--unrar            : unrar full path
	--unace            : unace full path
</pre>


### Licence
<pre>
//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
//
//
// ngPost is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 3.0 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301,
// USA.
//
//========================================================================
</pre>


### Questions / Issues / Requests
- if you've any troubles to build or run ex0days, feel free to drop me an email
- if you've some comments on the code, any questions on the implementation or any proposal for improvements, I'll be happy to discuss it with you so idem, feel free to drop me an email
- if you'd like some other features, same same (but different), drop me an email ;)

Here is my email: Matthieu.Bruel@gmail.com



### Thanks
- devuvo who ordered this soft


### Donations
I'm Freelance nowadays, working on several personal projects, so if you use the app and would like to contribute to the effort, feel free to donate what you can.<br/>
<br/>
[![](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=W2C236U6JNTUA&item_name=ex0days&currency_code=EUR)
<br/> or in Bitcoin on this address: **3BGbnvnnBCCqrGuq1ytRqUMciAyMXjXAv6** ![](https://raw.githubusercontent.com/mbruel/ngPost/master/pics/btc_qr.gif)
