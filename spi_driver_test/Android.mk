LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES:= spi_drv_test.c
LOCAL_MODULE:=silead_driver_test
include $(BUILD_EXECUTABLE)
