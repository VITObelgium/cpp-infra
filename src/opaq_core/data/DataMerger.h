/*
 * DataMerger.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef DATAMERGER_H_
#define DATAMERGER_H_

#include "DataStore.h"

namespace OPAQ {

class DataMerger: public OPAQ::DataStore {
public:
	DataMerger();
	virtual ~DataMerger();

	/**
	 * merge the model outputs into the target data store
	 */
	virtual void merge(DataStore const * const output) = 0;
};

} /* namespace OPAQ */
#endif /* DATAMERGER_H_ */
