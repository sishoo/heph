#pragma once


#define HEPH_THROW_ERROR_UNRECOVERABLE(err_msg) fprintf(stderr, "Unrecoverable Error: %s", err_msg); abort();