#ifndef RM_ERROR_H
#define RM_ERROR_H

#define RM_EOF             (START_RM_ERR)  // end of file
#define RM_FNOTOPEN        (START_RM_ERR - 1)
#define RM_NULLRECORD      (START_RM_ERR - 2)
#define RM_RID_PAGENUM_ERR (START_RM_ERR - 3)
#define RM_RID_SLOTNUM_ERR (START_RM_ERR - 4)

#endif // RM_ERROR_H