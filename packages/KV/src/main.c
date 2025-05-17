#include "recomp_api.h"
#include "z64extern.h"

#include "z64save.h"
#include "global.h"

#include "sys_flashrom.h"
#include "z64horse.h"
#include "overlays/gamestates/ovl_file_choose/z_file_select.h"


RECOMP_IMPORT(".", bool KV_PathUpdateInternal(unsigned char* key));
RECOMP_IMPORT(".", bool KV_Get(const char* key, void* dest, u32 size, u8 slot));
RECOMP_IMPORT(".", bool KV_Set(const char* key, void* data, u32 size, u8 slot));
RECOMP_IMPORT(".", bool KV_Has(const char* key, u8 slot));
RECOMP_IMPORT(".", bool KV_Remove(const char* key, u8 slot));
RECOMP_IMPORT(".", bool KV_DeleteSlot(u8 slot));
RECOMP_IMPORT(".", bool KV_CopySlot(u8 old_slot, u8 new_slot));


void KV_PathUpdate() {
    unsigned char* new_path = recomp_get_save_file_path();
    KV_PathUpdateInternal(new_path);
    recomp_free(new_path);
}

RECOMP_EXPORT bool KV_Global_Get(const char* key, void* dest, u32 size) {
    KV_PathUpdate();
    return KV_Get(key, dest, size, 255);
}

#define DEFINE_KV_GLOBAL_GETTER(type, suffix) \
RECOMP_EXPORT type KV_Global_Get_##suffix(const char* key, type defaultValue) { \
    KV_PathUpdate(); \
    type value = defaultValue; \
    KV_Get(key, &value, sizeof(type), 255); \
    return value; \
}

DEFINE_KV_GLOBAL_GETTER(u8, U8)
DEFINE_KV_GLOBAL_GETTER(s8, S8)
DEFINE_KV_GLOBAL_GETTER(u16, U16)
DEFINE_KV_GLOBAL_GETTER(s16, S16)
DEFINE_KV_GLOBAL_GETTER(u32, U32)
DEFINE_KV_GLOBAL_GETTER(s32, S32)

RECOMP_EXPORT bool KV_Global_Set(const char* key, void* data, u32 size) {
    KV_PathUpdate(); 
    return KV_Set(key, data, size, 255);
}

#define DEFINE_KV_GLOBAL_SETTER(type, suffix) \
RECOMP_EXPORT bool KV_Global_Set_##suffix(const char* key, type value) { \
    KV_PathUpdate(); \
    return KV_Set(key, &value, sizeof(type), 255); \
}

DEFINE_KV_GLOBAL_SETTER(u8, U8)
DEFINE_KV_GLOBAL_SETTER(s8, S8)
DEFINE_KV_GLOBAL_SETTER(u16, U16)
DEFINE_KV_GLOBAL_SETTER(s16, S16)
DEFINE_KV_GLOBAL_SETTER(u32, U32)
DEFINE_KV_GLOBAL_SETTER(s32, S32)

RECOMP_EXPORT bool KV_Global_Has(const char* key) {
    KV_PathUpdate(); 
    return KV_Has(key, 255);
}

RECOMP_EXPORT bool KV_Global_Remove(const char* key) {
    KV_PathUpdate(); 
    return KV_Remove(key, 255);
}

RECOMP_EXPORT bool KV_Slot_Get(const char* key, void* data, u32 size) {
    KV_PathUpdate(); 
    return KV_Get(key, data, size, gSaveContext.fileNum);
}

#define DEFINE_KV_SLOT_GETTER(type, suffix) \
RECOMP_EXPORT type KV_Slot_Get_##suffix(const char* key, type defaultValue) { \
    KV_PathUpdate(); \
    type value = defaultValue; \
    KV_Get(key, &value, sizeof(type), gSaveContext.fileNum); \
    return value; \
}

DEFINE_KV_SLOT_GETTER(u8, U8)
DEFINE_KV_SLOT_GETTER(s8, S8)
DEFINE_KV_SLOT_GETTER(u16, U16)
DEFINE_KV_SLOT_GETTER(s16, S16)
DEFINE_KV_SLOT_GETTER(u32, U32)
DEFINE_KV_SLOT_GETTER(s32, S32)

RECOMP_EXPORT bool KV_Slot_Set(const char* key, void* data, u32 size) {
    KV_PathUpdate(); 
    return KV_Set(key, data, size, gSaveContext.fileNum);
}

#define DEFINE_KV_SLOT_SETTER(type, suffix) \
RECOMP_EXPORT bool KV_Slot_Set_##suffix(const char* key, type value) { \
    KV_PathUpdate(); \
    return KV_Set(key, &value, sizeof(type), gSaveContext.fileNum); \
}

DEFINE_KV_SLOT_SETTER(u8, U8)
DEFINE_KV_SLOT_SETTER(s8, S8)
DEFINE_KV_SLOT_SETTER(u16, U16)
DEFINE_KV_SLOT_SETTER(s16, S16)
DEFINE_KV_SLOT_SETTER(u32, U32)
DEFINE_KV_SLOT_SETTER(s32, S32)

RECOMP_EXPORT bool KV_Slot_Has(const char* key) {
    KV_PathUpdate(); 
    return KV_Has(key, gSaveContext.fileNum);
}

RECOMP_EXPORT bool KV_Slot_Remove(const char* key) {
    KV_PathUpdate(); 
    return KV_Remove(key, gSaveContext.fileNum);
}

// Hook for deleting file save data

RECOMP_HOOK("Sram_EraseSave") void pre_Sram_EraseSave(FileSelectState* fileSelect2, SramContext* sramCtx, s32 fileNum) {
    KV_PathUpdate(); 
    KV_DeleteSlot((u8)fileNum);
}

RECOMP_HOOK("Sram_CopySave") void pre_Sram_CopySave(FileSelectState* fileSelect2, SramContext* sramCtx) {
    FileSelectState* fileSelect = fileSelect2;
    KV_PathUpdate(); 
    KV_CopySlot((u8)fileSelect->selectedFileIndex, (u8)fileSelect->copyDestFileIndex);
}
