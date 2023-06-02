//
// Programmer:    Katherine Wong
// Creation Date: Mon Mar 13 23:47:52 PDT 2023
// Last Modified: Mon Mar 13 23:47:55 PDT 2023
// Filename:      tool-colorthirds.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-colorthirds.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Color diatonic thirds in triadic sonorities:
//                    red   = root
//                    green = third
//                    blue  = fifth
//

#ifndef _TOOL_COLORTHIRDS_H
#define _TOOL_COLORTHIRDS_H

#include "HumTool.h"
#include "HumdrumFile.h"

namespace hum {

// START_MERGE

class Tool_colorthirds : public HumTool {
	public:
		         Tool_colorthirds  (void);
		        ~Tool_colorthirds  () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const string& indata, ostream& out);
		bool     run               (HumdrumFile& infile, ostream& out);

	protected:
		void             initialize        (void);
		void             processFile       (HumdrumFile& infile);
		std::vector<int> getMidiNotes(std::vector<HTp>& kernNotes);
		std::vector<int> getChordPositions(std::vector<int>& midiNotes);
		std::vector<int> getNoteMods(std::vector<int>& midiNotes);
		std::vector<int> getThirds(std::vector<int>& midiNotes);
		std::vector<int> getFifths(std::vector<int>& midiNotes);
		void             labelChordPositions(std::vector<HTp>& kernNotes, std::vector<int>& chordPositions);
		void             labelThirds(std::vector<HTp>& kernNotes, std::vector<int>& thirdPositions);
		void             labelFifths(std::vector<HTp>& kernNotes, std::vector<int>& fifthPositions);
		void             keepOnlyDoubles(std::vector<int>& output);
		void             checkForTriadicSonority(std::vector<int>& positions, int line);
		std::string      generateStatistics(HumdrumFile& infile);
		std::vector<std::string> getTrackNames(HumdrumFile& infile);
		int              getVectorSum(std::vector<int>& input);

	private:
		std::string m_root_marker = "@";
		std::string m_third_marker = "N";
		std::string m_fifth_marker = "Z";
		std::string m_3rd_root_marker = "j";
		std::string m_3rd_third_marker = "l";
		std::string m_5th_root_marker = "V";
		std::string m_5th_fifth_marker = "|";

		std::string m_root_color = "crimson";
		std::string m_third_color = "limegreen";
		std::string m_fifth_color = "royalblue";
		std::string m_3rd_root_color = "darkred";
		std::string m_3rd_third_color = "green";
		std::string m_5th_root_color = "darkred";
		std::string m_5th_fifth_color = "steelblue";

		bool m_colorThirds = true;
		bool m_colorFifths = true;
		bool m_colorTriads = true;
		bool m_doubleQ = false;

		// Statistical data variables:
		vector<bool> m_triadState;

		// m_partTriadPositions -- count the number of chordal positions by
		// voice.  The first dimention is the track number of the part, and
		// the second dimension is the counts for 7 categories:
		// 0 = count of root positions in full triadic chords
		// 1 = count of third positions in full triadic chords
		// 2 = count of fifth positions in full triadic chords
		// 3 = count of root positions in partial triadic chords ("open third")
		// 4 = count of third positions in partial triadic chords
		// 5 = count of root positions in partial triadic chords ("open fifth")
		// 6 = count of fifth positions in partial triadic chords
		std::vector<vector<int>> m_partTriadPositions;
		int m_positionCount = 7;

		string m_toolName = "colorthirds";

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_COLORTHIRDS_H */



