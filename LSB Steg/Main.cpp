/* This program hides the text in message.txt inside of input.bmp
   It will output to output.bmp

   Text is hidden using the least signifigant bit of each RGB byte of the image data.

   Source for .bmp file information: http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm

*/

#include<fstream>
#include<iostream>
#include<string>
#include<bitset>
using namespace std;

const int MAX_MSG = 30; //because I was lazy with not checking filesizes, max message length is limited here.

void inputToFile();
void outputToFile();

int main()
{
	int select;
	cout << "Input (1) or output (2)" << endl;
	cin >> select;

	if (select == 1)
	{
		inputToFile();
	}
	else
	{
		outputToFile();
	}
	return 0;
}


void inputToFile() //Hide the data
{
	string message; //String to be hidden
	unsigned char imageData[MAX_MSG * 8]; //Will store the bytes of image pixel data that will be used to hide the message.

	//Get the message 
	ifstream messageFile("message.txt");

	getline(messageFile, message);

	messageFile.close();

	// open the image file, and store it in a character array
	ifstream image("Test.bmp", ios::in | ios::binary);

	image.seekg(0x2);
	int fileSize;
	image.read(reinterpret_cast<char*>(&fileSize), sizeof(fileSize)); //get file size from the bmp's metadata

	int offset;
	image.seekg(0xa); //get start of pixel data location from metadata
	image.read(reinterpret_cast<char*>(&offset), sizeof(offset));
	
	unsigned char* file = new unsigned char[fileSize];

	image.seekg(0); //Pull the entire bmp file into RAM, include the header so you don't need to re-write it when saving.
	for (int i = 0; i < fileSize; i++)
	{
		file[i] = image.get();
	}
	image.close();

	for (int i = 0; i < (MAX_MSG * 8); i++) //copy the file's pixel data into a second array to make it simpler to edit. Note: this only copies the pixel data being modified
	{
		imageData[i] = file[offset + i];
	}

	//Actually do the data hiding
	for (int i = 0; i < (message.length() * 8); i++)
	{
		//message[i / 8] makes the loop run on the same index 8 times.
		//the ">> (i % 8)" part makes shifts from 0 to 7 on the above loop.
		//This selects each bit of a character, going from RIGHT TO LEFT, or LSB to MSB.
		unsigned char temp = message[i / 8] >> (i % 8); //extract each bit from the message


		//This next section asks first if the image pixel is odd or even, then asks if the bit you pulled is odd (1) or even(0)
		// Then, it checks to see if they are both even or odd. Which means you wouldn't have to change anything.
		if ((imageData[i] % 2) != temp % 2) //set the lsb of the imageData[i] to the message bit
		{
			if ((imageData[i] % 2) == 1) //If the image data is odd, subtract 1 to make it even. (Subtraction to prevent overflow: 255 + 1 = 0)
			{
				imageData[i] = imageData[i] - 1; 
			}
			else
			{
				imageData[i] = imageData[i] + 1; //if the image data is even, add 1 to make it odd. (Addition to prevent underflow: 0 - 1 = 255)
			}
		}
	}

	for (int i = 0; i < (message.length() * 8); i++) //copy updated data to file
	{
		file[i + offset] = imageData[i];
	}

	//Message is hidden, save to the output file.
	ofstream imageOut("output.bmp");

	for (int i = 0; i < fileSize; i++)
	{
		imageOut.put(file[i]);
	}
	imageOut.close();

	delete[] file;
}

void outputToFile() //Extract the data
{
	// open the image file, and find the first data bit.
	ifstream image("output.bmp");

	int offset;
	image.seekg(0xa); //0xa holds first data bit
	image.read(reinterpret_cast<char*>(&offset), sizeof(offset)); //get start of data

	image.seekg(offset);

	char message[MAX_MSG]; //message[] will hold the decoded message
	for (int i = 0; i < MAX_MSG; i++)
	{
		message[i] = 0;
	}

	//Pull out the lsb from each pixel color and re-construct the characters.
	for (int i = 0; i < (MAX_MSG * 8); i++)
	{
		unsigned char bit = image.get() % 2; //again asking: odd or even? 1 or 0?
		message[i / 8] = message[i/8] + (bit * pow(2, (i % 8))); //pow(2, (i % 8)) selects the bit you want to modify inside the character from right to left.
	}

	for (int i = 0; i < MAX_MSG; i++)
	{
		cout << message[i];
	}
}