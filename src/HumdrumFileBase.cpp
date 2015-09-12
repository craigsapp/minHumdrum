//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Fri Aug 14 21:57:09 PDT 2015
// Filename:      HumdrumFileBase.h
// URL:           https://github.com/craigsapp/minHumdrum/blob/master/src/HumdrumFileBase.h
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Used to store Humdrum text lines from input stream
//                for further parsing.  This class analyzes the basic
//                spine structure after reading a Humdrum file.  The
//                HumdrumFileStructure class continues structural analysis,
//                primarily of rhythm (generated by **kern, **recip and
//                **koto data) and global/local parameters.
//

#include "HumdrumFileBase.h"

#include <sstream>
#include <fstream>
#include "stdarg.h"

namespace minHumdrum {

// START_MERGE


//////////////////////////////
//
// HumdrumFileBase::HumdrumFileBase -- HumdrumFileBase constructor.
//

HumdrumFileBase::HumdrumFileBase(void) {
	addToTrackStarts(NULL);
	ticksperquarternote = -1;
	quietParse = false;
}

HumdrumFileBase::HumdrumFileBase(const string& filename) {
	addToTrackStarts(NULL);
	ticksperquarternote = -1;
	quietParse = false;
	read(filename);
}

HumdrumFileBase::HumdrumFileBase(istream& contents) {
	addToTrackStarts(NULL);
	ticksperquarternote = -1;
	quietParse = false;
	read(contents);
}



//////////////////////////////
//
// HumdrumFileBase::~HumdrumFileBase -- HumdrumFileBase deconstructor.
//

HumdrumFileBase::~HumdrumFileBase() { }



//////////////////////////////
//
// HumdrumFileBase::setXmlIdPrefix -- Set the prefix for a HumdrumXML ID
//     atrribute.  The prefix should not start with a digit, nor have
//     spaces in it.
//

void HumdrumFileBase::setXmlIdPrefix(const string& value) {
	idprefix = value;
}



//////////////////////////////
//
// HumdrumFileBase::getXmlIdPrefix -- Return the HumdrumXML ID attribute prefix.
//

string HumdrumFileBase::getXmlIdPrefix(void) {
	return idprefix;
}



//////////////////////////////
//
// HumdrumFileBase::operator[] -- Access a Humdrum file line by and index.
//    Negative values reference the end of the list of lines.
//

HumdrumLine& HumdrumFileBase::operator[](int index) {
	if (index < 0) {
		index = lines.size() - index;
	}
	if ((index < 0) || (index >= lines.size())) {
		cerr << "Error: invalid index: " << index << endl;
		index = lines.size()-1;
	}
	return *lines[index];
}



//////////////////////////////
//
// HumdrumFileBase::setParseError -- Set an error message from parsing
//     input data.  The size of the message will keep track of whether
//     or not an error was generated.  If no error message is generated
//     when reading data, then the parsing of the data is assumed to be
//     good.
//

bool HumdrumFileBase::setParseError(const string& err) {
	parseError = err;
	return !parseError.size();
}


bool HumdrumFileBase::setParseError(stringstream& err) {
	return setParseError(err.str());
}


bool HumdrumFileBase::setParseError(const char* format, ...) {
	char buffer[1024] = {0};	
	va_list ap;
	va_start(ap, format);
	snprintf(buffer, 1024, format, ap);
	va_end(ap);
	return setParseError(buffer);
}



//////////////////////////////
//
// HumdrumFileBase::read -- Load file contents from an input stream or file.
//

bool HumdrumFileBase::read(const string& filename) {
	return HumdrumFileBase::read(filename.c_str());
}


bool HumdrumFileBase::read(const char* filename) {
	ifstream infile;
	if ((strlen(filename) == 0) || (strcmp(filename, "-") == 0)) {
		return HumdrumFileBase::read(cin);
	} else {
		infile.open(filename);
		if (!infile.is_open()) {
			return setParseError("Cannot open file %s for reading.", filename);
		}
	}
	HumdrumFileBase::read(infile);
	infile.close();
	return isValid();
}


bool HumdrumFileBase::read(istream& contents) {
	char buffer[123123] = {0};
	HumdrumLine* s;
	while (contents.getline(buffer, sizeof(buffer), '\n')) {
		s = new HumdrumLine(buffer);
		s->setOwner(this);
		lines.push_back(s);
	}
	if (!analyzeTokens()         ) { return isValid(); }
	if (!analyzeLines()          ) { return isValid(); }
	if (!analyzeSpines()         ) { return isValid(); }
	if (!analyzeLinks()          ) { return isValid(); }
	if (!analyzeTracks()         ) { return isValid(); }
	return isValid();
}



//////////////////////////////
//
// HumdrumFileBase::readCsv -- Read a Humdrum file in CSV format
//    (rather than TSV format).
// default value: separator = ","
//

bool HumdrumFileBase::readCsv(const string& filename, const string& separator) {
	return HumdrumFileBase::readCsv(filename.c_str());
}


bool HumdrumFileBase::readCsv(const char* filename, const string& separator) {
	ifstream infile;
	if ((strlen(filename) == 0) || (strcmp(filename, "-") == 0)) {
		return HumdrumFileBase::readCsv(cin, separator);
	} else {
		infile.open(filename);
		if (!infile.is_open()) {
			return setParseError("Cannot open file %s for reading.", filename);
		}
	}
	HumdrumFileBase::readCsv(infile, separator);
	infile.close();
	return isValid();
}


bool HumdrumFileBase::readCsv(istream& contents, const string& separator) {
	char buffer[123123] = {0};
	HumdrumLine* s;
	while (contents.getline(buffer, sizeof(buffer), '\n')) {
		s = new HumdrumLine;
		s->setLineFromCsv(buffer);
		s->setOwner(this);
		lines.push_back(s);
	}
	if (!analyzeTokens()         ) { return isValid(); }
	if (!analyzeLines()          ) { return isValid(); }
	if (!analyzeSpines()         ) { return isValid(); }
	if (!analyzeLinks()          ) { return isValid(); }
	if (!analyzeTracks()         ) { return isValid(); }
	return isValid();
}



//////////////////////////////
//
// HumdrumFileBase::readString -- Read contents from a string rather than
//    an istream or filename.
//

bool HumdrumFileBase::readString(const string& contents) {
	stringstream infile;
	infile << contents;
	return read(infile);
}


bool HumdrumFileBase::readString(const char* contents) {
	stringstream infile;
	infile << contents;
	return read(infile);
}



//////////////////////////////
//
// HumdrumFileBase::readStringCsv -- Reads Humdrum data in CSV format.
//

bool HumdrumFileBase::readStringCsv(const char* contents,
		const string& separator) {
	stringstream infile;
	infile << contents;
	return readCsv(infile, separator);
}


bool HumdrumFileBase::readStringCsv(const string& contents,
		const string& separator) {
	stringstream infile;
	infile << contents;
	return readCsv(infile, separator);
}



//////////////////////////////
//
// HumdrumFileBase::isValid -- Returns true if last read was 
//     successful.
//

bool HumdrumFileBase::isValid(void) const {
   return !parseError.size();
}



//////////////////////////////
//
// HumdrumFileBase::setQuietParse -- Prevent error messages from
//   being displayed when reading data.
//

void HumdrumFileBase::setQuietParse(void) {
	quietParse = false;
}



//////////////////////////////
//
// HumdrumFileBase::setNoisyParse -- Display  error messages 
//   on console when reading data.
//

void HumdrumFileBase::setNoisyParse(void) {
	quietParse = true;
}



//////////////////////////////
//
// HumdrumFileBase::printCsv -- print Humdrum file content in
//     CSV format.
// default value: out = std::cout
// default value: separator = ","
//

ostream& HumdrumFileBase::printCsv(ostream& out,
		const string& separator) {
	for (int i=0; i<getLineCount(); i++) {
		((*this)[i]).printCsv(out, separator);
	}
	return out;
}



//////////////////////////////
//
// HumdrumFileBase::analyzeTokens -- Generate token array from
//    current contents of the lines.  If either tokens or the line
//    is changed, then the other state becomes invalid.
//    See createLinesFromTokens for regeneration of lines from tokens.
//

bool HumdrumFileBase::analyzeTokens(void) {
	int i;
	for (i=0; i<lines.size(); i++) {
		lines[i]->createTokensFromLine();
	}
	return isValid();
}



//////////////////////////////
//
// HumdrumFileBase::createLinesFromTokens -- Generate Humdrum lines strings
//   from the stored list of tokens.
//

void HumdrumFileBase::createLinesFromTokens(void) {
	for (int i=0; i<lines.size(); i++) {
		lines[i]->createLineFromTokens();
	}
}



////////////////////////////
//
// HumdrumFileBase::append -- Add a line to the file's contents.  The file's
//    spine and rhythmic structure should be recalculated after an append.
//

void HumdrumFileBase::append(const char* line) {
	HumdrumLine* s = new HumdrumLine(line);
	lines.push_back(s);
}


void HumdrumFileBase::append(const string& line) {
	HumdrumLine* s = new HumdrumLine(line);
	lines.push_back(s);
}



////////////////////////////
//
// HumdrumFileBase::getLineCount -- Returns the number of lines.
//

int HumdrumFileBase::getLineCount(void) const {
	return lines.size();
}



//////////////////////////////
//
// HumdrumFileBase::token -- Return the token at the given line/field index.
//

HumdrumToken& HumdrumFileBase::token(int lineindex, int fieldindex) {
	if (lineindex < 0) {
		lineindex += getLineCount();
	}
	return lines[lineindex]->token(fieldindex);
}



//////////////////////////////
//
// HumdrumFileBase::getMaxTrack -- Returns the number of primary
//     spines in the data.
//

int HumdrumFileBase::getMaxTrack(void) const {
	return trackstarts.size() - 1;
}



//////////////////////////////
//
// HumdrumFileBase::printSpineInfo -- Print the spine information for all
//    lines/tokens in file (for debugging).
//

ostream& HumdrumFileBase::printSpineInfo(ostream& out) {
	for (int i=0; i<getLineCount(); i++) {
		lines[i]->printSpineInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFileBase::printDataTypeInfo -- Print the data type for all
//     spines in the file (for debugging).
//

ostream& HumdrumFileBase::printDataTypeInfo(ostream& out) {
	for (int i=0; i<getLineCount(); i++) {
		lines[i]->printDataTypeInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFileBase::printTrackInfo -- Print the track numbers for all
//     tokens in the file (for debugging).
//

ostream& HumdrumFileBase::printTrackInfo(ostream& out) {
	for (int i=0; i<getLineCount(); i++) {
		lines[i]->printTrackInfo(out) << '\n';
	}
	return out;
}



//////////////////////////////
//
// HumdrumFileBase::getPrimaryTrackSeq -- Return a list of the
//     given primary spine tokens for a given track (indexed starting at
//     one and going through getMaxTrack().
//
// The following options are used for the getPrimaryTrackTokens.
// * OPT_NONULLS    => don't include  null tokens in extracted list
// * OPT_NOMANIP    => don't include  spine manipulators (*^, *v, *x, *+,
//                        but still keep ** and *0).
// * OPT_NOGLOBALS  => don't include global records (global comments, reference
//                        records, and empty lines). In other words, only return
//                        a list of tokens from lines which hasSpines() it true.
//
// #define OPT_NONULLS  0x01
// #define OPT_NOMANIP  0x02
// #define OPT_NOGLOBAL 0x04
//

vector<HumdrumToken*> HumdrumFileBase::getPrimaryTrackSeq(int track,
		int options) {
	int i;
	int nullQ    = (options & OPT_NONULLS);
	int manipQ   = (options & OPT_NOMANIP);
	int globalQ  = (options & OPT_NOGLOBAL);
	vector<HumdrumToken*> output;
	output.reserve(getLineCount());

	auto current = getTrackStart(track);
	if (current == NULL) {
		return output;
	}
	HumdrumFileBase& infile = *this;
	int startindex = current->getLineIndex();
	if (globalQ) {
		for (i=0; i<startindex; i++) {
			if (!infile[i].hasSpines()) {
				output.push_back(&(infile[i].token(0)));
			}
		}
	}

	while (current != NULL) {
		if ((!nullQ) && current->isNull()) {
			// don't insert
		} else if ((!manipQ) && current->isManipulator() &&
				(!current->isTerminator()) && (!current->isExclusive())) {
			// don't insert
		} else {
			if ((output.size() > 0) && globalQ) {
				for (i=output.back()->getLineIndex();i<current->getLineIndex();i++){
					if (!infile[i].hasSpines()) {
						output.push_back(&infile[i].token(0));
					}
				}
			}
			output.push_back(current);
		}
		if (current->getNextTokenCount() > 0) {
			current = current->getNextToken();
		} else {
			current = NULL;
			break;
		}
		
	}

	int endindex = output.back()->getLineIndex();
	if (globalQ) {
		for (i=endindex; i<infile.getLineCount(); i++) {
			if (infile[i].hasSpines()) {
				continue;
			}
			output.push_back(&infile[i].token(0));
		}
	}
	return output;
}



/////////////////////////////
//
// HumdrumFileBase::getTrackSeq -- Extract a sequence of tokens
//    for the given spine.  All subspine tokens will be included.
//    See getPrimaryTrackSeq() if you only want the first subspine for
//    a track on all lines.
//

vector<vector<HumdrumToken*> > HumdrumFileBase::getTrackSeq(int track,
		int options) {
	int nullQ    = (options & OPT_NONULLS);
	int manipQ   = (options & OPT_NOMANIP);
	int globalQ  = (options & OPT_NOGLOBAL);

	vector<vector<HumdrumToken*> > output;
	output.reserve(getLineCount());

	vector<HumdrumToken*> tempout;
	auto& infile = *this;
	int i, j;

	for (i=0; i<infile.getLineCount(); i++) {
		tempout.resize(0);
		if (globalQ && (!infile[i].hasSpines())) {
			output.push_back(tempout);
			continue;
		}
		for (j=0; j<infile[i].getFieldCount(); j++) {
			if (infile[i].token(j).getTrack() == track) {
				if ((!nullQ) && infile[i].token(j).isNull()) {
					continue;
				}
				if ((!nullQ) && infile[i].token(j).isNull()) {
					continue;
				} else if ((!manipQ) && infile[i].token(j).isManipulator() &&
						(!infile[i].token(j).isTerminator()) &&
						(!infile[i].token(j).isExclusive())) {
					continue;
				}
				tempout.push_back(&infile[i].token(j));
			}
		}
		if (tempout.size() > 0) {
			output.push_back(tempout);
		}
	}
	return output;
}



//////////////////////////////
//
// HumdrumFileBase::getTrackStart -- Return the starting exclusive
//     interpretation for the given track.  Returns NULL if the track
//     number is out of range.
//

HumdrumToken* HumdrumFileBase::getTrackStart(int track) const {
	if ((track > 0) && (track < trackstarts.size())) {
		return trackstarts[track];
	} else {
		return NULL;
	}
}



//////////////////////////////
//
// HumdrumFileBase::getTrackEndCount -- Return the number of ending tokens
//    for the given track.  Spines must start as a single exclusive
//    interpretation token.  However, since spines may split and merge,
//    it is possible that there are more than one termination points for a
//    track.  This function returns the number of terminations which are
//    present in a file for any given spine/track.
//

int HumdrumFileBase::getTrackEndCount(int track) const {
	if (track < 0) {
		track += trackends.size();
	}
	if (track < 0) {
		return 0;
	}
	if (track >= trackends.size()) {
		return 0;
	}
	return trackends[track].size();
}



//////////////////////////////
//
// HumdrumFileBase::getTrackEnd -- Returns a pointer to the terminal manipulator
//    token for the given track and subtrack.  Sub-tracks are indexed from 0 up
//    to but not including getTrackEndCount.
//

HumdrumToken* HumdrumFileBase::getTrackEnd(int track, int subtrack) const {
	if (track < 0) {
		track += trackends.size();
	}
	if (track < 0) {
		return NULL;
	}
	if (track >= trackends.size()) {
		return NULL;
	}
	if (subtrack < 0) {
		subtrack += trackends[track].size();
	}
	if (subtrack < 0) {
		return NULL;
	}
	if (subtrack >= trackends[track].size()) {
		return NULL;
	}
	return trackends[track][subtrack];
}



//////////////////////////////
//
// HumdrumFileBase::analyzeLines -- Store a line's index number in the
//    HumdrumFile within the HumdrumLine object at that index.
//    Returns false if there was an error.
//

bool HumdrumFileBase::analyzeLines(void) {
	for (int i=0; i<lines.size(); i++) {
		lines[i]->setLineIndex(i);
	}
	return isValid();
}



//////////////////////////////
//
// HumdrumFileBase::analyzeTracks -- Analyze the track structure of the
//     data.  Returns false if there was a parse error.
//

bool HumdrumFileBase::analyzeTracks(void) {
	for (int i=0; i<lines.size(); i++) {
		int status = lines[i]->analyzeTracks(parseError);
		if (!status) {
			return false;
		}
	}
	return isValid();
}



//////////////////////////////
//
// HumdrumFileBase::analyzeLinks -- Generate forward and backwards spine links
//    for each token.
//

bool HumdrumFileBase::analyzeLinks(void) {
	HumdrumLine* next     = NULL;
	HumdrumLine* previous = NULL;

	for (int i=0; i<lines.size(); i++) {
		if (!lines[i]->hasSpines()) {
			continue;
		}
		previous = next;
		next = lines[i];
		if (previous != NULL) {
			if (!stitchLinesTogether(*previous, *next)) {
				return isValid();
			}
		}
	}
	return isValid();;
}



//////////////////////////////
//
// HumdrumFileBase::stitchLinesTogether -- Make forward/backward links for
//    tokens on each line.
//

bool HumdrumFileBase::stitchLinesTogether(HumdrumLine& previous,
		HumdrumLine& next) {
	int i;

	// first handle simple cases where the spine assignments are one-to-one:
	if (!previous.isInterpretation() && !next.isInterpretation()) {
		if (previous.getTokenCount() != next.getTokenCount()) {
			stringstream err;
			err << "Error lines " << (previous.getLineNumber())
			    << " and " << (next.getLineNumber()) << " not same length\n";
			err << "Line " << (previous.getLineNumber()) << ": "
			    << previous << endl;
			err << "Line " << (next.getLineNumber()) << ": "
			    << next;
			return setParseError(err);
		}
		for (i=0; i<previous.getTokenCount(); i++) {
			previous.token(i).makeForwardLink(next.token(i));
		}
		return true;
	}
	int ii = 0;
	for (i=0; i<previous.getTokenCount(); i++) {
		if (!previous.token(i).isManipulator()) {
			previous.token(i).makeForwardLink(next.token(ii++));
		} else if (previous.token(i).isSplitInterpretation()) {
			// connect the previous token to the next two tokens.
			previous.token(i).makeForwardLink(next.token(ii++));
			previous.token(i).makeForwardLink(next.token(ii++));
		} else if (previous.token(i).isMergeInterpretation()) {
			// connect multiple previous tokens which are adjacent *v
			// spine manipulators to the current next token.
			while ((i<previous.getTokenCount()) &&
					previous.token(i).isMergeInterpretation()) {
				previous.token(i).makeForwardLink(next.token(ii));
				i++;
			}
			i--;
			ii++;
		} else if (previous.token(i).isExchangeInterpretation()) {
			// swapping the order of two spines.
			if ((i<previous.getTokenCount()) &&
					previous.token(i+1).isExchangeInterpretation()) {
				previous.token(i+1).makeForwardLink(next.token(ii++));
				previous.token(i).makeForwardLink(next.token(ii++));
			}
			i++;
		} else if (previous.token(i).isTerminateInterpretation()) {
			// No link should be made.  There may be a problem if a
			// new segment is given (this should be handled by a
			// HumdrumSet class, not HumdrumFileBase.
		} else if (previous.token(i).isAddInterpretation()) {
			// A new data stream is being added, the next linked token
			// should be an exclusive interpretation.
			if (!next.token(ii+1).isExclusiveInterpretation()) {
				stringstream err;
				err << "Error: expecting exclusive interpretation on line "
				    << next.getLineNumber() << " at token " << i << " but got "
				    << next.token(i) << endl;
				return setParseError(err);
			}
			previous.token(i).makeForwardLink(next.token(ii++));
			ii++;
		} else if (previous.token(i).isExclusiveInterpretation()) {
			previous.token(i).makeForwardLink(next.token(ii++));
		} else {
			return setParseError("Error: should not get here");
		}
	}

	if ((i != previous.getTokenCount()) || (ii != next.getTokenCount())) {
		stringstream err;
		err << "Error: cannot stitch lines together due to alignment problem\n";
		err << "Line " << previous.getLineNumber() << ": "
		    << previous << endl;
		err << "Line " << next.getLineNumber() << ": "
		    << next << endl;
		err << "I = " <<i<< " token count " << previous.getTokenCount() << endl;
		err << "II = " <<ii<< " token count " << next.getTokenCount() << endl;
		return setParseError(err);
	}

	return isValid();
}



//////////////////////////////
//
// HumdrumFileBase::analyzeSpines -- Analyze the spine structure of the
//     data.  Returns false if there was a parse error.
//

bool HumdrumFileBase::analyzeSpines(void) {
	vector<string> datatype;
	vector<string> sinfo;
	vector<vector<HumdrumToken*> > lastspine;
	trackstarts.resize(0);
	trackends.resize(0);
	addToTrackStarts(NULL);

	bool init = false;
	int i, j;
	for (i=0; i<getLineCount(); i++) {
		if (!lines[i]->hasSpines()) {
			lines[i]->token(0).setFieldIndex(0);
			continue;
		}
		if ((init == false) && !lines[i]->isExclusive()) {
			stringstream err;
			err << "Error on line: " << (i+1) << ':' << endl;
			err << "   Data found before exclusive interpretation" << endl;
			err << "   LINE: " << *lines[i];
			return setParseError(err);
		}
		if ((init == false) && lines[i]->isExclusive()) {
			// first line of data in file.
			init = true;
			datatype.resize(lines[i]->getTokenCount());
			sinfo.resize(lines[i]->getTokenCount());
			lastspine.resize(lines[i]->getTokenCount());
			for (j=0; j<lines[i]->getTokenCount(); j++) {
				datatype[j] = lines[i]->getTokenString(j);
				addToTrackStarts(&lines[i]->token(j));
				sinfo[j]    = to_string(j+1);
				lines[i]->token(j).setSpineInfo(sinfo[j]);
				lines[i]->token(j).setFieldIndex(j);
				lastspine[j].push_back(&(lines[i]->token(j)));
			}
			continue;
		}
		if (datatype.size() != lines[i]->getTokenCount()) {
			stringstream err;
			err << "Error on line " << (i+1) << ':' << endl;
			err << "   Expected " << datatype.size() << " fields,"
			     << " but found " << lines[i]->getTokenCount() << endl;
			return setParseError(err);
		}
		for (j=0; j<lines[i]->getTokenCount(); j++) {
			lines[i]->token(j).setSpineInfo(sinfo[j]);
			lines[i]->token(j).setFieldIndex(j);
		}
		if (!lines[i]->isManipulator()) {
			continue;
		}
		if (!adjustSpines(*lines[i], datatype, sinfo)) { return isValid(); }
	}
	return isValid();
}



//////////////////////////////
//
// HumdrumFileBase::addToTrackStarts -- A starting exclusive interpretation was
//    found, so store in the list of track starts.  The first index position
//    in trackstarts is reserve for non-spine usage.
//

void HumdrumFileBase::addToTrackStarts(HumdrumToken* token) {
	if (token == NULL) {
		trackstarts.push_back(NULL);
		trackends.resize(trackends.size()+1);
	} else if ((trackstarts.size() > 1) && (trackstarts.back() == NULL)) {
		trackstarts.back() = token;
	} else {
		trackstarts.push_back(token);
		trackends.resize(trackends.size()+1);
	}
}



//////////////////////////////
//
// HumdrumFileBase::adjustSpines -- adjust datatype and spineinfo values based
//   on manipulators found in the data.
//

bool HumdrumFileBase::adjustSpines(HumdrumLine& line, vector<string>& datatype,
		vector<string>& sinfo) {
	vector<string> newtype;
	vector<string> newinfo;
	int mergecount = 0;
	int i, j;
	for (i=0; i<line.getTokenCount(); i++) {
		if (line.token(i).isSplitInterpretation()) {
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newinfo.resize(newinfo.size() + 2);
			newinfo[newinfo.size()-2] = '(' + sinfo[i] + ")a";
			newinfo[newinfo.size()-1] = '(' + sinfo[i] + ")b";
		} else if (line.token(i).isMergeInterpretation()) {
			mergecount = 0;
			for (j=i+1; j<line.getTokenCount(); j++) {
				if (line.token(j).isMergeInterpretation()) {
					mergecount++;
				} else {
					break;
				}
			}
			newinfo.resize(newtype.size() + 1);
			newinfo.back() = getMergedSpineInfo(sinfo, i, mergecount);
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			i += mergecount;
		} else if (line.token(i).isAddInterpretation()) {
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newtype.resize(newtype.size() + 1);
			newtype.back() = "";
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
			newinfo.resize(newinfo.size() + 1);
			addToTrackStarts(NULL);
			newinfo.back() = to_string(getMaxTrack());
		} else if (line.token(i).isExchangeInterpretation()) {
			if (i < line.getTokenCount() - 1) {
				if (line.token(i).isExchangeInterpretation()) {
					// exchange spine information
					newtype.resize(newtype.size() + 1);
					newtype.back() = datatype[i+1];
					newtype.resize(newtype.size() + 1);
					newtype.back() = datatype[i];
					newinfo.resize(newinfo.size() + 1);
					newinfo.back() = sinfo[i+1];
					newinfo.resize(newinfo.size() + 1);
					newinfo.back() = sinfo[i];
				} else {
					return setParseError("ERROR1 in *x calculation");
				}
				i++;
			} else {
				stringstream err;
				err << "ERROR2 in *x calculation" << endl;
				err << "Index " << i << " larger than allowed: "
				     << line.getTokenCount() - 1 << endl;
				return setParseError(err);
			}
		} else if (line.token(i).isTerminateInterpretation()) {
			// store pointer to terminate token in trackends
			trackends[trackstarts.size()-1].push_back(&(line.token(i)));
		} else if (((string)line.token(i)).substr(0, 2) == "**") {
			newtype.resize(newtype.size() + 1);
			newtype.back() = line.getTokenString(i);
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
			if (!((trackstarts.size() > 1) && (trackstarts.back() == NULL))) {
				stringstream err;
				err << "Error: Exclusive interpretation with no preparation "
				     << "on line " << line.getLineIndex()
				     << " spine index " << i << endl;
				err << "Line: " << line << endl;
				return setParseError(err);
			}
			if (trackstarts.back() == NULL) {
				addToTrackStarts(&line.token(i));
			}
		} else {
			// should only be null interpretation, but doesn't matter
			newtype.resize(newtype.size() + 1);
			newtype.back() = datatype[i];
			newinfo.resize(newinfo.size() + 1);
			newinfo.back() = sinfo[i];
		}
	}

	datatype.resize(newtype.size());
	sinfo.resize(newinfo.size());
	for (i=0; i<newtype.size(); i++) {
		datatype[i] = newtype[i];
		sinfo[i]    = newinfo[i];
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileBase::getMergedSpineInfo -- Will only simplify a two-spine
//   merge.  Should be expanded to larger spine mergers in the future.
//   In other words, it is best to currently merge spines in the order
//   in which they were split, so that the original spine label can
//   be produced.
//

string HumdrumFileBase::getMergedSpineInfo(vector<string>& info, int starti,
		int extra) {
	string output;
	int len1;
	int len2;
	if (extra == 1) {
		len1 = info[starti].size();
		len2 = info[starti+1].size();
		if (len1 == len2) {
			if (info[starti].substr(0, len1-1) ==
					info[starti+1].substr(0,len2-1)) {
				output = info[starti].substr(1, len1-3);
				return output;
			}
		}
		output = info[starti] + " " + info[starti+1];
		return output;
	}
	output = info[starti];
	for (int i=0; i<extra; i++) {
		output += " " + info[starti+1+extra];
	}
	return output;
}



//////////////////////////////
//
// HumdrumFileBase::analyzeNonNullDataTokens -- For null data tokens, indicate
//    the previous non-null token which the null token refers to.  After
//    a spine merger, there may be multiple previous tokens, so you would
//		have to decide on the actual source token on based on subtrack or
//    sub-spine information.  The function also gives links to the previous/next
//    non-null tokens, skipping over intervening null data tokens.
//

bool HumdrumFileBase::analyzeNonNullDataTokens(void) {
	vector<HumdrumToken*> ptokens;

	// analyze forward tokens:
	for (int i=1; i<=getMaxTrack(); i++) {
		if (!processNonNullDataTokensForTrackForward(getTrackStart(i),
				ptokens)) {
			return false;
		}
	}

	// analyze backward tokens:
	for (int i=1; i<=getMaxTrack(); i++) {
		for (int j=0; j<getTrackEndCount(i); j++) {
			if (!processNonNullDataTokensForTrackBackward(getTrackEnd(i, j),
					ptokens)) {
				return false;
			}
		}
	}
	return true;
}



//////////////////////////////
//
// HumdurmFile::processNonNullDataTokensForTrackBackward -- Helper function
//    for analyzeNonNullDataTokens.  Given any token, this function tells
//    you what is the next non-null data token(s) in the spine after the given
//    token.
//

bool HumdrumFileBase::processNonNullDataTokensForTrackBackward(
		HumdrumToken* endtoken, vector<HumdrumToken*> ptokens) {

	HumdrumToken* token = endtoken;
	int tcount = token->getPreviousTokenCount();
	while (tcount > 0) {
		for (int i=1; i<tcount; i++) {
			if (!processNonNullDataTokensForTrackBackward(
					token->getPreviousToken(i), ptokens)) {
				return false;
			}
		}
		if (token->isData()) {
			addUniqueTokens(token->nextNonNullTokens, ptokens);
			if (!token->isNull()) {
				ptokens.resize(0);
				ptokens.push_back(token);
			}
		}
		// Data tokens can only be followed by up to one previous token,
		// so no need to check for more than one next token.
		token = token->getPreviousToken(0);
		tcount = token->getPreviousTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdurmFile::processNonNullDataTokensForTrackForward -- Helper function
//    for analyzeNonNullDataTokens.  Given any token, this function tells
//    you what is the previous non-null data token(s) in the spine after
//    the given token.
//

bool HumdrumFileBase::processNonNullDataTokensForTrackForward(
		HumdrumToken* starttoken, vector<HumdrumToken*> ptokens) {
	HumdrumToken* token = starttoken;
	int tcount = token->getNextTokenCount();
	while (tcount > 0) {
		if (!token->isData()) {
			for (int i=1; i<tcount; i++) {
				if (!processNonNullDataTokensForTrackForward(
						token->getNextToken(i), ptokens)) {
					return false;
				}
			}
		} else {
			addUniqueTokens(token->previousNonNullTokens, ptokens);
			if (!token->isNull()) {
				ptokens.resize(0);
				ptokens.push_back(token);
			}
		}
		// Data tokens can only be followed by up to one next token,
		// so no need to check for more than one next token.
		token = token->getNextToken(0);
		tcount = token->getNextTokenCount();
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileBase::addUniqueTokens -- Used for non-null token analysis.  The
//    analysis is recursive like rhythmic analysis in the HumdrumFileStructure
//    class, but this algorithm does not terminate secondary traversals when
//    recursing.  Perhaps that should be fixed (utilizing the "rhycheck"
//    variable in HumdrumTokens)
//

void HumdrumFileBase::addUniqueTokens(vector<HumdrumToken*>& target,
		vector<HumdrumToken*>& source) {
	int i, j;
	bool found;
	for (i=0; i<source.size(); i++) {
		found = false;
		for (j=0; j<target.size(); j++) {
			if (source[i] == target[i]) {
				found = true;
			}
		}
		if (!found) {
			target.push_back(source[i]);
		}
	}
}



//////////////////////////////
//
// operator<< -- Default method of printing HumdrumFiles.  This printing method
//    assumes that the HumdrumLine string is correct.  If a token is changed
//    in the file, the HumdrumFileBase::createLinesFromTokens() before printing
//    the contents of the line.
//

ostream& operator<<(ostream& out, HumdrumFileBase& infile) {
	for (int i=0; i<infile.getLineCount(); i++) {
		out << infile[i] << '\n';
	}
	return out;
}


//////////////////////////////
//
// sortTokenParisByLineIndex -- Sort two tokens so that the one
//    on the smaller line is first.  If both are on the same line then
//    sort the left-most token first.
//

bool sortTokenPairsByLineIndex(const TokenPair& a, const TokenPair& b) {
	if (a.first->getLineIndex() < b.first->getLineIndex()) {
		return true;
	}
	if (a.first->getLineIndex() == b.first->getLineIndex()) {
		if (a.first->getFieldIndex() < b.first->getFieldIndex()) {
			return true;
		}
	}
	return false;
}



// END_MERGE

} // end namespace std;
