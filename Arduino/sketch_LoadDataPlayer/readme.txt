This sketch allows a DataPlayer to be initialized and loaded with image data.
Use the files in lvl_DPcontrol/DPimages in this distro.
Copy and paste data using the Serial monitor.
It's a bit sensitive. Sometimes it goes wrong. Please improve if you see how.
I may be processing the Serial data wrong/poorly so that it will only work just so.
However, it's loading 16 color 16x16 images well.
A full list of the byte values read and their sum is displayed so the data
can be checked against known values.

The file format:
3 sections:

upper block = 3 lines
rows cols
row0 col0
stepPause drawOff fadeAlong

2nd block lists the colors in the image
numColors + 1 lines:
numColors
numColorsx(red green blue)

3rd block = image data
dataInBits// bool: false if data given in byte values
number of index values to follow.
Blocks of index values in groups of rows*cols.

Example: A 16x16 image of a bed. Data is from file:
lvl_DPcontrol/DPimages/bed16color16x16_data.txt
Shown here:

16 16
0 0
80 1 0

11
255 255 255
183 144 11
142 110 0
190 216 216
149 163 163
211 243 243
0 146 146
135 105 2
106 85 11
167 194 194
0 113 113

1
256
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 1 2 0 0 0 0 0 0 0 0 0 0 0 0 1 
1 0 2 0 0 0 0 0 0 0 0 0 0 0 1 2 
2 0 2 3 0 0 0 0 0 0 0 0 0 1 0 2 
2 0 3 3 3 0 0 0 0 0 0 0 0 2 0 2 
2 4 3 3 3 3 5 6 6 6 6 6 6 2 6 2 
2 7 4 3 3 5 6 6 6 6 6 6 6 2 1 2 
2 5 5 4 5 6 6 6 6 6 6 6 6 2 8 2 
2 9 9 9 9 10 10 10 10 10 10 10 10 2 8 2 
2 9 9 9 9 10 10 10 10 10 10 10 10 2 0 2 
2 9 9 9 9 10 10 10 10 10 10 10 10 2 0 8 
2 0 8 0 0 10 10 10 10 10 10 10 10 2 0 8 
2 0 8 0 0 0 0 0 0 0 0 0 0 2 0 8 
2 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0 
2 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0

I will run it through the program.
Start Arduino. Get the prompt:

Ready for data

I will paste the data in each block separately.

1st paste:

16 16
0 0
80 1 0

Output in monitor:

Ready for data
Rows = 16
Cols = 16
Row0 = 0
Col0 = 0
stepPause = 80
drawOff = 1
fadeAlong = 0
Serial.avail = 0

2nd paste is the color list

11
255 255 255
183 144 11
142 110 0
190 216 216
149 163 163
211 243 243
0 146 146
135 105 2
106 85 11
167 194 194
0 113 113

New output:

numColors = 11
color: 255, 255, 255
color: 183, 144, 11
color: 142, 110, 0
color: 190, 216, 216
color: 149, 163, 163
color: 211, 243, 243
color: 0, 146, 146
color: 135, 105, 2
color: 106, 85, 11
color: 167, 194, 194
color: 0, 113, 113
Serial.avail = 0

Last paste is the remaining data:

1
256
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 
0 1 2 0 0 0 0 0 0 0 0 0 0 0 0 1 
1 0 2 0 0 0 0 0 0 0 0 0 0 0 1 2 
2 0 2 3 0 0 0 0 0 0 0 0 0 1 0 2 
2 0 3 3 3 0 0 0 0 0 0 0 0 2 0 2 
2 4 3 3 3 3 5 6 6 6 6 6 6 2 6 2 
2 7 4 3 3 5 6 6 6 6 6 6 6 2 1 2 
2 5 5 4 5 6 6 6 6 6 6 6 6 2 8 2 
2 9 9 9 9 10 10 10 10 10 10 10 10 2 8 2 
2 9 9 9 9 10 10 10 10 10 10 10 10 2 0 2 
2 9 9 9 9 10 10 10 10 10 10 10 10 2 0 8 
2 0 8 0 0 10 10 10 10 10 10 10 10 2 0 8 
2 0 8 0 0 0 0 0 0 0 0 0 0 2 0 8 
2 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0 
2 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0

The output is a bit voluminous:

k = 240   Serial.avail = 0
16 Done! k = 256
Serial.avail = 0

 initDataPlay(): BA.bitSize() = 1024
Player load Good!
numBytes = 128

0	0	0	0	0	0	0	0	0	8	
0	0	0	0	0	0	128	4	0	0	
0	0	0	128	8	4	0	0	0	0	
0	72	4	196	0	0	0	0	128	64	
4	204	12	0	0	0	64	64	36	204	
204	106	102	102	70	70	228	194	172	102	
102	102	70	72	164	42	106	102	102	102	
70	65	148	153	89	85	85	85	69	65	
148	153	89	85	85	85	69	64	148	153	
89	85	85	85	69	16	4	1	80	85	
85	85	69	16	4	1	0	0	0	0	
64	16	4	0	0	0	0	0	64	0	
4	0	0	0	0	0	64	0	
 chkSum = 6714

These values agree with the images in use in the main PC project.
For animations with multiple blocks of data enter each block of rows*cols values separately. The data in some files can be pasted 4 blocks at a time
( blue guy ) but in other cases it will parse wrong.
