/*
 * rmi.h
 *
 *  Created on: 4Aug.,2019
 *      Author: betocool
 */

#ifndef RMI_H_
#define RMI_H_

#define NANOPB_BUFFER_LEN		1024
#define RMI_DEBUG_BUFFER_LEN	256

#include "analyser.pb.h"
#include "pb_common.h"
#include "pb_decode.h"
#include "pb_encode.h"

void rmi_init(void);

#endif /* RMI_H_ */
