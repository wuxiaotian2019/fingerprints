/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <dirent.h>

#include "fpc_log.h"
#include "fpc_tee_sensor.h"
#include "fpc_tee.h"
#include "fpc_tee_sensortest.h"
#include "fpc_tee_bio.h"
#include "fpc_tee_kpi.h"
#include "fpc_types.h"
#include "fpc_error_str.h"
#include "fpc_tee_engineering.h"

#include "fpc_tee_db_blob_test.h"

#define DEBUG_PATH_BASE     "/data/system"
#define DEBUG_PATH_ENROL    DEBUG_PATH_BASE"/enroll/"
#define DEBUG_PATH_VERIFY   DEBUG_PATH_BASE"/verify/"
#define DEBUG_TEST_DB       DEBUG_PATH_BASE"/test.db"

#if defined(FPC_CONFIG_ENGINEERING) && defined(FPC_CONFIG_SENSORTEST)
int sensor_test(fpc_tee_sensor_t* sensor, uint32_t *result)
{
    int status = fpc_tee_sensortest_run_test(sensor, FPC_TEE_SENSORTEST_SELF_TEST, result);
    printf("%s: %d\n", __func__, *result);
    return status;
}

int checkerboard_test(fpc_tee_sensor_t* sensor, uint32_t *result)
{
    int status = fpc_tee_sensortest_run_test(sensor, FPC_TEE_SENSORTEST_CHECKERBOARD_TEST, result);
    printf("%s: %d\n", __func__, *result);
    return status;
}

int imagequality_test(fpc_tee_sensor_t* sensor, uint32_t *result)
{
    printf("Put rubber stamp against sensor...\n");
    int status = fpc_tee_sensortest_run_test(sensor, FPC_TEE_SENSORTEST_IMAGE_QUALITY_TEST, result);
    printf("%s: %d\n", __func__, *result);
    return status;
}

int resetpixel_test(fpc_tee_sensor_t* sensor, uint32_t *result)
{
    int status = fpc_tee_sensortest_run_test(sensor, FPC_TEE_SENSORTEST_RESET_PIXEL_TEST, result);
    printf("%s: %d\n", __func__, *result);
    return status;
}

int capture_uncalibrated(fpc_tee_sensor_t* sensor)
{
    int status = fpc_tee_sensortest_capture_uncalibrated(sensor);
    printf("%s: %d\n", __func__, status);
    return status;
}

int auto_inject_image(fpc_tee_t* tee, size_t sensor_size, DIR **directory, char *debugpath)
{
    int ret = 0;
    size_t read_result = 0;
    uint8_t file_buffer[sensor_size];
    FILE* image_file = NULL;
    char filename[100];
    struct dirent* file;
    size_t len = 0;

    if (*directory == NULL) {
        LOGE("%s Error opening directory", __func__);
        return -1;
    }

    while ((file=readdir(*directory)) != NULL) {
        len = strlen(file->d_name);
        if (len >= 4 && memcmp(file->d_name + len - 4, ".raw", 4) == 0) {
            strcpy(filename, debugpath);
            strcat(filename, file->d_name);
            image_file = fopen(filename, "r");
            read_result = fread(file_buffer, 1, sensor_size, image_file);
            if (read_result != sensor_size) {
                LOGE("%s Entire file not read", __func__);
                fclose(image_file);
                return -1;
            }
            ret = fpc_tee_debug_inject(tee, file_buffer, sensor_size);
            fclose(image_file);
            return ret;
        }
    }
    /* Not enough images, use same images again. */
    closedir(*directory);
    *directory = opendir(debugpath);
    return 1;
}

#endif
int enroll_test(fpc_tee_sensor_t* sensor, fpc_tee_bio_t* bio, fpc_tee_t* tee,
                int8_t no_sensor)
{
    int status = 0;
    uint32_t remaining_samples = 0;
    size_t sensor_size = 0;
    uint8_t width = 0;
    uint8_t height = 0;
    uint32_t id = 0;
    uint64_t authenticator_id;
    char debugpath[50] = DEBUG_PATH_ENROL;
    DIR *directory;
    directory = opendir(debugpath);

    fpc_tee_kpi_start(tee);

    status = fpc_tee_begin_enrol(bio);
    if (status) {
        goto out;
    }

    if (no_sensor) {
#if defined(FPC_CONFIG_ENGINEERING) && defined(FPC_CONFIG_SENSORTEST)
        status = fpc_tee_get_sensor_info(tee, &width, &height);
        if(status) {
            LOGD("%s could not get sensor_size", __func__);
            goto out;
        }
        sensor_size = (size_t) width*height;
#endif
    } else {
        printf("Put finger against sensor repeatedly...\n");
    }
    for (;;) {
        if (no_sensor) {
#if defined(FPC_CONFIG_ENGINEERING) && defined(FPC_CONFIG_SENSORTEST)
            status = auto_inject_image(tee, sensor_size, &directory, debugpath);
            if (status == 1) {
                status = auto_inject_image(tee, sensor_size, &directory, debugpath);
            }
#endif
        } else {
            status = fpc_tee_capture_image(sensor);
            if (status == FPC_CAPTURE_QUALIFY_ABORT) {
                // This will happen after exceeded maximum attempts
                // of trying to capture an image.
                continue;
            }
        }
        if (status) {
            goto out;
        }

        status = fpc_tee_enrol(bio, &remaining_samples);
        if (status < 0) {
            goto out;
        }

        switch (status) {
        case FPC_ENROL_COMPLETED:
            status  = fpc_tee_end_enrol(bio, &id);
            if (status) {
                goto out;
            }

            status = fpc_tee_store_template_db(bio, DEBUG_TEST_DB);

            if (status) {
                printf("Store template failed\n");
                fpc_tee_load_template_db(bio, DEBUG_TEST_DB);
                goto out;
            }
            status = fpc_tee_get_template_db_id(bio, &authenticator_id);

            if (status) {
                LOGE("%s failed to get auth id %i\n", __func__, status);
                printf("Failed to get auth id %i\n", status);
                authenticator_id = 0;
            }

            status = 0;
            printf("Completed\n");
            goto out;
        case FPC_ENROL_PROGRESS:
            LOGD("%s acquired fingerprint\n", __func__);
            printf("Acquired fingerprint\n");
            break;
        case FPC_ENROL_FAILED_COULD_NOT_COMPLETE:
            LOGE("%s unable to process fingerprint %i\n", __func__, status);
            printf("Error: Unable to process\n");
            status = 0;
            goto out;
        case FPC_ENROL_IMAGE_TOO_SIMILAR:
            LOGE("%s got a too similar fingerprint %i\n", __func__, status);
            printf("Too similar fingerprint\n");
            break;
        case FPC_ENROL_FAILED_ALREADY_ENROLED:
            LOGE("%s is already enrolled %i\n", __func__, status);
            printf("Already enrolled\n");
            status = 0;
            goto out;
        case FPC_ENROL_IMAGE_LOW_QUALITY:
            LOGE("%s got image with too low quality %i\n", __func__, status);
            printf("Too low quality\n");
            break;
        case FPC_ENROL_IMAGE_LOW_COVERAGE:
            LOGE("%s got image with too low coverage %i\n", __func__, status);
            printf("Too low coverage\n");
            break;
        }
    }
out:
    if (status) {
        LOGE("%s failed %s\n", __func__, fpc_error_str(status));
        switch (status){
        case -FPC_ERROR_CANCELLED:
            break;
        case -FPC_ERROR_TIMEDOUT:
            printf("Timed out\n");
            break;
        case -FPC_ERROR_NOSPACE:
            printf("No space\n");
            break;
        default:
            printf("Error HW unavailable\n");
            break;
        }
    }
    closedir(directory);
    fpc_tee_kpi_stop(tee);
    printf("%s: %d\n", __func__, status);
    return status;
}

int authentication_test(fpc_tee_sensor_t* sensor, fpc_tee_bio_t* bio, fpc_tee_t* tee,
                        int8_t no_sensor)
{
    LOGD("%s", __func__);
    size_t sensor_size = 0;
    uint8_t width = 0;
    uint8_t height = 0;
    int status = 0;
    uint32_t id;
    uint32_t update = 0;
    char debugpath[50] = DEBUG_PATH_VERIFY;
    DIR *directory;
    directory = opendir(debugpath);

    fpc_tee_kpi_start(tee);

#if defined(FPC_CONFIG_ENGINEERING) && defined(FPC_CONFIG_SENSORTEST)
    if (no_sensor) {
        status = fpc_tee_get_sensor_info(tee, &width, &height);
        if(status){
            LOGD("%s could not get sensor_size", __func__);
            goto out;
        }
        sensor_size = (size_t) width*height;
    }
#endif

    for (;;) {
        if (no_sensor) {
#if defined(FPC_CONFIG_ENGINEERING) && defined(FPC_CONFIG_SENSORTEST)
            status = auto_inject_image(tee, sensor_size, &directory, debugpath);
            if (status == 1) {
                status = auto_inject_image(tee, sensor_size, &directory, debugpath);
            }
#endif
        } else {
            printf("Put finger against sensor...\n");
            status = fpc_tee_capture_image(sensor);
            if (status == FPC_CAPTURE_QUALIFY_ABORT) {
                // This will happen after exceeded maximum attempts
                // of trying to capture an image.
                continue;
           }
        }

        if (status != FPC_CAPTURE_OK) {
            goto out;
        }

        LOGD("%s acquired fingerprint\n", __func__);
        printf("Acquired fingerprint\n");

        status = fpc_tee_identify(bio, &id);
        if (status) {
            goto out;
        }

        if (id != 0) {
            LOGD("%s authenticated fingerprint\n", __func__);
            printf("Authenticated fingerprint\n");

            status = fpc_tee_update_template(bio, &update);
            if (status) {
                goto out;
            }

            if (update != 0) {
                fpc_tee_store_template_db(bio, DEBUG_TEST_DB);
            }
        } else {
            LOGD("%s did not authenticate fingerprint\n", __func__);
            printf("Did not authenticate fingerprint\n");
        }
    }

out:
    if (status) {
        LOGE("%s failed %s\n", __func__, fpc_error_str(status));
        switch (status) {
        case -FPC_ERROR_CANCELLED:
            break;
        default:
            printf("Error HW unavailable\n");
            break;
        }
    }
    closedir(directory);
    fpc_tee_kpi_stop(tee);
    printf("%s: %d\n", __func__, status);
    return status;
}

int afd_calibration_test(fpc_tee_sensor_t* sensor, uint32_t *result)
{
    int status = fpc_tee_sensortest_run_test(sensor, FPC_TEE_SENSORTEST_AFD_CALIBRATION_TEST, result);
    printf("%s: %d\n", __func__, *result);
    return status;
}

int afd_calibration_rubber_stamp_test(fpc_tee_sensor_t* sensor, uint32_t *result)
{
    printf("Put rubber stamp against sensor...\n");
    int status = fpc_tee_sensortest_run_test(sensor, FPC_TEE_SENSORTEST_AFD_CALIBRATION_RUBBER_STAMP_TEST, result);
    printf("%s: %d\n", __func__, *result);
    return status;
}

int afd_rubber_stamp_test(fpc_tee_sensor_t* sensor, uint32_t *result)
{
    printf("Put rubber stamp against sensor...\n");
    int status = fpc_tee_sensortest_run_test(sensor, FPC_TEE_SENSORTEST_AFD_RUBBER_STAMP_TEST, result);
    printf("%s: %d\n", __func__, *result);
    return status;
}

int usage(char *pname)
{
  printf(
    "%s usage:\n"
#ifndef FPC_CONFIG_NO_SENSOR
#if defined(FPC_CONFIG_ENGINEERING) && defined(FPC_CONFIG_SENSORTEST)
    "-s, --sensor_test                          Runs sensor test\n"
    "-b, --checkerboard_test                    Runs checkerboard_test\n"
    "-q, --imagequality_test                    Runs imagequality_test\n"
    "-p, --resetpixel_test                      Runs resetpixel_test\n"
    "-u, --capture_uncalibrated                 Runs capture_uncalibrated\n"
    "-C, --afd_calibration_test                 Runs afd_calibration_test\n"
    "-r, --afd_calibration_rubber_stamp_test    Runs afd_calibration_rubber_stamp_test\n"
    "-R, --afd_rubber_stamp_test                Runs afd_rubber_stamp_test\n"
#endif
    "-e, --enroll_test                          Runs enroll_test\n"
    "-a, --authentication_test                  Runs authentication_test\n"
    "-d, --db_blob_test                         Runs db_blob_test\n"
#endif
#if defined(FPC_CONFIG_ENGINEERING) && defined(FPC_CONFIG_SENSORTEST)
    "-E, --enroll_no_sensor_test                Runs enroll_no_sensor_test\n"
    "-A, --authentication_no_sensor_test        Runs authentication_no_sensor_test\n"
#endif
    , pname);

  return -1;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        return usage(argv[0]);
    }

    uint32_t result = 0;
    int opt = 0;
    int long_index = 0;
    int status = -1;
    int8_t no_sensor = 0;

    fpc_tee_t* tee = fpc_tee_init();
    if(!tee) {
        LOGE("%s , error creating tee_handle.", __func__);
        return -1;
    }

    fpc_tee_sensor_t* sensor = fpc_tee_sensor_init(tee);
    if (!sensor) {
        LOGE("%s , error creating sensor_handle.", __func__);
        return -1;
    }

    fpc_tee_bio_t* bio = fpc_tee_bio_init(tee);
    status = fpc_tee_print_build_info(tee);
    if (status) {
        LOGD("%s, An error occurred print build info.\n", __func__);
    }

    static struct option long_options[] =
    {
        {"sensor_test",                       no_argument,    0,  's'},
        {"checkerboard_test",                 no_argument,    0,  'b'},
        {"imagequality_test",                 no_argument,    0,  'q'},
        {"resetpixel_test",                   no_argument,    0,  'p'},
        {"capture_uncalibrated",              no_argument,    0,  'u'},
        {"enroll_test",                       no_argument,    0,  'e'},
        {"enroll_no_sensor_test",             no_argument,    0,  'E'},
        {"authentication_test",               no_argument,    0,  'a'},
        {"db_blob_test",                      no_argument,    0,  'd'},
        {"authentication_no_sensor_test",     no_argument,    0,  'A'},
        {"afd_calibration_test",              no_argument,    0,  'C'},
        {"afd_calibration_rubber_stamp_test", no_argument,    0,  'r'},
        {"afd_rubber_stamp_test",             no_argument,    0,  'R'},
        {0,                                   0,              0,    0}
    };

    while ((opt = getopt_long(argc, argv,"sbqcpueEadACrR",
        long_options, &long_index )) != -1)
    {
        switch(opt) {
#ifndef FPC_CONFIG_NO_SENSOR
#if defined(FPC_CONFIG_ENGINEERING) && defined(FPC_CONFIG_SENSORTEST)
        case 's':
            printf("Starting sensor_test\n");
            status = sensor_test(sensor, &result);
            break;
        case 'b':
            printf("Starting checkerboard_test\n");
            status = checkerboard_test(sensor, &result);
            break;
        case 'q':
            printf("Starting imagequality_test\n");
            status = imagequality_test(sensor, &result);
            break;
        case 'p':
            printf("Starting resetpixel_test\n");
            status = resetpixel_test(sensor, &result);
            break;
        case 'u':
            printf("Starting capture_uncalibrated\n");
            status = capture_uncalibrated(sensor);
            printf("End of test.\n");
            return status;
        case 'C': //Supported by fpc1022, fpc1023, fpc1270
            printf("Starting afd_calibration_test\n");
            status = afd_calibration_test(sensor, &result);
            break;
        case 'r': //Supported by fpc1022, fpc1023, fpc1260, fpc1270
            printf("Starting afd_calibration_rubber_stamp_test\n");
            status = afd_calibration_rubber_stamp_test(sensor, &result);
            break;
        case 'R': //Supported by fpc1022
            printf("Starting afd_rubber_stamp_test\n");
            status = afd_rubber_stamp_test(sensor, &result);
            break;
#endif
        case 'e':
            printf("Starting enroll_test\n");
            status = enroll_test(sensor, bio, tee, no_sensor);
            break;
        case 'a':
            printf("Starting authentication_test\n");
            enroll_test(sensor, bio, tee, no_sensor);
            status = authentication_test(sensor, bio, tee, no_sensor);
            break;
        case 'd':
            printf("Starting db blob test\n");
            status = test_fpc_tee_db_blob(tee);
            break;
#endif
#if defined(FPC_CONFIG_ENGINEERING) && defined(FPC_CONFIG_SENSORTEST)
        case 'E':
            printf("Starting enroll_no_sensor test\n");
            no_sensor = 1;
            status = enroll_test(sensor, bio, tee, no_sensor);
            break;
        case 'A':
            printf("Starting authentication_no_sensor test\n");
            no_sensor = 1;
            enroll_test(sensor, bio, tee, no_sensor);
            status = authentication_test(sensor, bio, tee, no_sensor);
            break;
#endif
        default:
            printf("Non handled option '%c'. Close down\n", opt);
            fpc_tee_bio_release(bio);
            fpc_tee_sensor_release(sensor);
            fpc_tee_release(tee);
            return usage(argv[0]);
        }
    }

    fpc_tee_bio_release(bio);
    fpc_tee_sensor_release(sensor);
    fpc_tee_release(tee);
    if (status != 0) {
        LOGE("%s could not perform test\n", __func__);
        return -1;
    }

    printf("End of test\n");
    return (int) result;
}
