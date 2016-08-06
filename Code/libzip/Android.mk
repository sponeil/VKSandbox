include $(CLEAR_VARS)

LOCAL_MODULE    := libzip
LOCAL_SRC_FILES :=\
    libzip/zip_add.c \
    libzip/zip_add_dir.c \
    libzip/zip_close.c \
    libzip/zip_delete.c \
    libzip/zip_dirent.c \
    libzip/zip_entry_free.c \
    libzip/zip_entry_new.c \
    libzip/zip_err_str.c \
    libzip/zip_error.c \
    libzip/zip_error_clear.c \
    libzip/zip_error_get.c \
    libzip/zip_error_get_sys_type.c \
    libzip/zip_error_strerror.c \
    libzip/zip_error_to_str.c \
    libzip/zip_fclose.c \
    libzip/zip_file_error_clear.c \
    libzip/zip_file_error_get.c \
    libzip/zip_file_get_offset.c \
    libzip/zip_file_strerror.c \
    libzip/zip_filerange_crc.c \
    libzip/zip_fopen.c \
    libzip/zip_fopen_index.c \
    libzip/zip_fread.c \
    libzip/zip_free.c \
    libzip/zip_get_archive_comment.c \
    libzip/zip_get_archive_flag.c \
    libzip/zip_get_file_comment.c \
    libzip/zip_get_num_files.c \
    libzip/zip_get_name.c \
    libzip/zip_memdup.c \
    libzip/zip_name_locate.c \
    libzip/zip_new.c \
    libzip/zip_open.c \
    libzip/zip_rename.c \
    libzip/zip_replace.c \
    libzip/zip_set_archive_comment.c \
    libzip/zip_set_archive_flag.c \
    libzip/zip_set_file_comment.c \
    libzip/zip_source_buffer.c \
    libzip/zip_source_file.c \
    libzip/zip_source_filep.c \
    libzip/zip_source_free.c \
    libzip/zip_source_function.c \
    libzip/zip_source_zip.c \
    libzip/zip_set_name.c \
    libzip/zip_stat.c \
    libzip/zip_stat_index.c \
    libzip/zip_stat_init.c \
    libzip/zip_strerror.c \
    libzip/zip_unchange.c \
    libzip/zip_unchange_all.c \
    libzip/zip_unchange_archive.c \
    libzip/zip_unchange_data.c

LOCAL_LDLIBS := -lz

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
