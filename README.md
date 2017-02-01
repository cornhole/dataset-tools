# Cornhole Vision Dataset Tools

## Compile
1. Install [OpenCV 3.2] (http://docs.opencv.org/2.4/doc/tutorials/introduction/linux_install/linux_install.html#linux-installation)
2. Install Boost libraries:
 * `sudo apt-get install libboost1.58-dev`
 * `sudo apt-get install libboost-filesystem1.58-dev`

3. Clone the jsoncpp repository into this folder
 * `cd jsoncpp`
 * `python amalgamate.py`
 
4. Clone the dataset repository into this folder

5. `make`

## Run
1. Likely need to set environmental variables to include shared OpenCV libraries
 * `export LD_LIBRARY_PATH=/usr/local/lib`
2. To capture new data: `./captureNewImage`
3. To parse existing: `./parseJson`

## Author

Tyler Frasca<br />
tmfrasca@gmail.com<br />
http://tmfrasca.info
