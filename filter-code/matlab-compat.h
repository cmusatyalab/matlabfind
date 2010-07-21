/* a few functions, enums, and typedefs for building against MATLAB */

#ifndef MATLAB_COMPAT_H
#define MATLAB_COMPAT_H


typedef struct a Engine;
typedef struct b mxArray;
typedef struct c MATFile;

extern Engine *engOpen(const char *arg0);
extern int engClose(Engine *arg0);
extern int engPutVariable(Engine *arg0, const char *arg1, const mxArray *arg2);
extern int engEvalString(Engine *arg0, const char *arg1);
extern mxArray *engGetVariable(Engine *arg0, const char *arg1);

extern size_t mxGetNumberOfDimensions(const mxArray *arg0);
extern void mxDestroyArray(mxArray *arg0);

typedef enum {
  mxUNKNOWN_CLASS,
  mxCELL_CLASS,
  mxSTRUCT_CLASS,
  mxLOGICAL_CLASS,
  mxCHAR_CLASS,
  mxVOID_CLASS,
  mxDOUBLE_CLASS,
  mxSINGLE_CLASS,
  mxINT8_CLASS,
  mxUINT8_CLASS,
  mxINT16_CLASS,
  mxUINT16_CLASS,
  mxINT32_CLASS,
  mxUINT32_CLASS,
  mxINT64_CLASS,
  mxUINT64_CLASS,
  mxFUNCTION_CLASS
} mxClassID;

typedef enum {
  mxREAL,
  mxCOMPLEX
} mxComplexity;

extern mxClassID mxGetClassID(const mxArray *arg0);
extern const size_t *mxGetDimensions(const mxArray *arg0);
extern void *mxGetData(const mxArray *arg0);
extern mxArray *mxCreateNumericArray(size_t arg0, const size_t *arg1, mxClassID arg2, mxComplexity arg3);

extern MATFile *matOpen(const char *arg0, const char *arg1);
extern mxArray *matGetNextVariable(MATFile *arg0, const char **arg1);
extern int matClose(MATFile *arg0);

extern size_t mxGetNumberOfElements(const mxArray *arg0);
extern double *mxGetPr(const mxArray *arg0);


#endif
