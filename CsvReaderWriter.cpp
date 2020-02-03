// CsvReaderWriter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <memory>

using namespace std;

///////////////////////////////////////////////////////////////////////////
// Helper templates to help convert string fields to/from other data types.
// These examples are simplistic but they can be expanded to perfrom more
// complex conversions depending on your needs.  You can also define your
// own custom data types and write the conversion functions for them.
///////////////////////////////////////////////////////////////////////////
template<typename T>
struct myStringConverter {};

template<> struct myStringConverter<string>
{
	static void from_string_converter(const string& s, string& t) { t = s; }
	static string to_string_converter(const string& s) { return s;  }
};

template<> struct myStringConverter<int>
{
	static void from_string_converter(const string& s, int& t) { t = stoi(s); }
	static string to_string_converter(const int& s) { return to_string(s); }
};

template<> struct myStringConverter<long>
{
	static void from_string_converter(const string& s, long& t) { t = stol(s); }
	static string to_string_converter(const long& s) { return to_string(s); }
};

template<> struct myStringConverter<long long>
{
	static void from_string_converter(const string& s, long long& t) { t = stoll(s); }
	static string to_string_converter(const long long& s) { return to_string(s); }
};

template<> struct myStringConverter<float>
{
	static void from_string_converter(const string& s, float& t) { t = stof(s); }
	static string to_string_converter(const float& s) { return to_string(s); }
};

template<> struct myStringConverter<double>
{
	static void from_string_converter(const string& s, double& t) { t = stof(s); }
	static string to_string_converter(const double& s) { return to_string(s); }
};

template<> struct myStringConverter<long double>
{
	static void from_string_converter(const string& s, long double& t) { t = stold(s); }
	static string to_string_converter(const long double& s) { return to_string(s); }
};

/////////////////////////////////////////////////////////////////////
// Abstract class for reading/writing data to/from a data file.
/////////////////////////////////////////////////////////////////////
class DataIO
{
protected:
	string fileName;
	ifstream input_file;
	ofstream output_file;
	bool file_ready;

public:
	bool isReady() { return file_ready; }
	virtual bool getData(vector<string>& dataList) = 0;
	virtual bool putData(const vector<string>& dataList) = 0;
};

////////////////////////////////////////////
// Class to read data from a CSV file.
////////////////////////////////////////////
class CSVReader : public DataIO
{
	char delimeter;

public:
	CSVReader(string filename, char delm = ',') : delimeter(delm)
	{
		fileName = filename;
		input_file.open(filename);
		if (!input_file)
		{
			cout << "CSVReader(): Error opening " << fileName << endl;
			file_ready = false;
		}
		else
		{
			cout << "CSVReader(): Successfully opened file " << filename << endl;
			file_ready = true;
		}
	}
	~CSVReader() { if (file_ready) input_file.close(); }

	// Function to fetch data from a CSV File
	bool getData(vector<string>& dataList);
	bool putData(const vector<string>& dataList) { return false; }
};


// Parses through csv file line by line and returns the data in vector of strings.
bool CSVReader::getData(vector<string>& dataList)
{
	if (!file_ready)
		return false;

	string line, word;
	dataList.clear();

	// Read the line and split the content using delimeter
	getline(input_file, line);
	stringstream s(line);
	while (getline(s, word, delimeter))
	{
		dataList.push_back(word);
	}

	if (dataList.size())
		return true;
	else
		return false;
}

/////////////////////////////////////////////////
// Class to write data to a csv file.
/////////////////////////////////////////////////
class CSVWriter : public DataIO
{
	char delimeter;

public:
	CSVWriter(string filename, char delm = ',') :
		delimeter(delm)
	{
		fileName = filename;
		output_file.open(filename, ios::out | ios::trunc);
		if (!output_file.is_open())
		{
			cout << "CSVWriter(): Error opening " << fileName << endl;
			file_ready = false;
		}
		else
		{
			cout << "CSVWriter(): Successfully opened file " << filename << endl;
			file_ready = true;
		}
	}
	~CSVWriter() { if (file_ready) output_file.close(); }

	bool getData(vector<string>& dataList) { return false; }

	// Function to write data to a CSV File
	bool putData(const vector<string>& dataList);
};

// Takes a vector of strings and write them to the data file.
bool CSVWriter::putData(const vector<string>& dataList)
{
	if (!file_ready)
		return false;

	for (size_t i = 0; i < dataList.size(); ++i)
	{
		if (i)
			output_file << delimeter << dataList[i];
		else
			output_file << dataList[i];
	}
	output_file << endl;

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// Security Information structure used to hold statistics for each security.
//////////////////////////////////////////////////////////////////////////////
template<typename P, typename Q, typename W, typename T>
struct SecurityInfo
{
	T max_time_gap;
	T last_time_stamp;
	Q total_volume;
	P max_trade_price;
	W wap_price_qty;

	SecurityInfo() : max_time_gap{}, last_time_stamp{}, total_volume{}, max_trade_price{}, wap_price_qty{} {}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Driver class which does the following:
// 1) Method which uses the reader objects to read the securities data from the input file (loadData())
// 2) Saves the security statistics information on a per symbol basis in a map (sec_list)
// 3) Methos which uses the writer objects to write the statistis information to the output file (printStats())
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename S, typename P, typename Q, typename W, typename T>
class SecurityStats
{
	map<S, shared_ptr<SecurityInfo<P,Q,W,T> > > sec_list;
	DataIO *input_reader;
	DataIO *output_writer;

public:
	SecurityStats(DataIO* ireader, DataIO* owriter) : input_reader(ireader), output_writer(owriter) {  }
	bool loadData();
	void printStats();
};


// Uses the reader object to read data from the input file and saves the informaiton in map
template<typename S, typename P, typename Q, typename W, typename T>
bool SecurityStats<S,P,Q,W,T>::loadData()
{
	size_t counter(0);
	vector<string> dataList;
	while (input_reader->getData(dataList))
	{
		T time_stamp{};
		S symbol;
		Q trade_quantity{};
		P trade_price{};

		if (dataList.size() != 4)
			continue;

		for (int i = 0; i < (int)dataList.size(); ++i)
		{
			switch (i)
			{
				case 0: // TimeStamp
				{
					myStringConverter<T>::from_string_converter(dataList[i], time_stamp);
					break;
				}
				case 1: // Symbol
				{
					myStringConverter<S>::from_string_converter(dataList[i], symbol);
					break;
				}
				case 2: // Quantity
				{
					myStringConverter<Q>::from_string_converter(dataList[i], trade_quantity);
					break;
				}
				case 3: // Price
				{
					myStringConverter<P>::from_string_converter(dataList[i], trade_price);
					break;
				}
			}
		}
		counter++;

		// Lookup symbol in security list
		auto itr = sec_list.find(symbol);
		shared_ptr<SecurityInfo<P,Q,W,T> > secptr;
		if (itr == sec_list.end())
		{
			// Security does not exist so create new SecurityInfo for this security.
			secptr = make_shared < SecurityInfo<P,Q,W,T> >();
			sec_list[symbol] = secptr;
		}
		else
			secptr = itr->second;

		// Check for new Maximum Time Gap
		if (secptr->last_time_stamp)
		{
			long long time_diff = time_stamp - secptr->last_time_stamp;
			if (time_diff > secptr->max_time_gap)
				secptr->max_time_gap = time_diff;
		}
		secptr->last_time_stamp = time_stamp;

		// Update total volume
		secptr->total_volume += trade_quantity;

		// Update Max Trade Price
		if (trade_price > secptr->max_trade_price)
			secptr->max_trade_price = trade_price;

		// Update values to calculate WAP
		secptr->wap_price_qty += ((W)trade_price * (W)trade_quantity);
	}

	if (sec_list.empty())
		return false;

	cout << "SecurityStats::loadData(): Read " << counter << " trade records for " << sec_list.size() << " securities" << endl;
	return true;
}

// Uses the writer object to write the statistics data to the output file
template<typename S, typename P, typename Q, typename W, typename T>
void SecurityStats<S, P, Q, W, T>::printStats()
{
	size_t counter(0);

	// Iterate through security list and write its statistics to the file.
	for (std::pair<S, shared_ptr<SecurityInfo<P,Q,W,T> > > element : sec_list) 
	{
		// Get security info struct for this security
		shared_ptr<SecurityInfo<P,Q,W,T> > secptr = element.second;

		// Calculate Weighted Average Price for this security
		W wap = secptr->wap_price_qty / secptr->total_volume;

		// Write the statistics for this security to file.
		vector<string> dataList;
		dataList.push_back(myStringConverter<S>::to_string_converter(element.first));
		dataList.push_back(myStringConverter<T>::to_string_converter(secptr->max_time_gap));
		dataList.push_back(myStringConverter<Q>::to_string_converter(secptr->total_volume));
		dataList.push_back(myStringConverter<W>::to_string_converter(wap));
		dataList.push_back(myStringConverter<P>::to_string_converter(secptr->max_trade_price));
		if (output_writer->putData(dataList))
			counter++;
	}

	cout << "SecurityStats::putData(): Wrote " << counter << " statistical records for " << sec_list.size() << " securities" << endl;
}

int main()
{
	// Instatiate reader and writer objects
	CSVReader csv_reader("/tmp/input.csv");
	CSVWriter csv_writer("/tmp/output.csv");

	// Instantiate SecurityStats object  and call loadData() and printStats()
	SecurityStats<string, int, int, long long, long long> secstats(&csv_reader, &csv_writer);
	if (secstats.loadData())
		secstats.printStats();
	else
		cout << "No DATA to print - check input file" << endl;
}


