/*
 * GzipReader.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef GZIPREADER_H_
#define GZIPREADER_H_

#include <gzstream.h>
#include "../Exceptions.h"
#include <sstream>
#include "FileTools.h"

namespace OPAQ {

  /**
     Filereader based upon igzstream
     See: http://www.cs.unc.edu/Research/compgeom/gzstream/
  */
  class GzipReader {
  public:
    GzipReader();
    virtual ~GzipReader();

    /** Open the gzip file
      * Throws IOException */
    void open (const std::string & filename);

    /** Reads a line from the file and returns as a std::string */
    std::string readLine();

    /** filepointer is at the end of the file */
    bool eof();
    
    /** close the input file */
    void close();
    
  private:
    igzstream * _igzstream;
    
  };
  
} /* namespace OPAQ */
#endif /* GZIPREADER_H_ */
