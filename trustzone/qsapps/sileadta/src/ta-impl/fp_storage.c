#define FILE_TAG "fp_store"
#include "silead_logmsg.h"

#include "silead_base.h"
#include "silead_error.h"
#include "silead_fp_storage_impl.h"
#include "fp_util.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

#define FNAME_MAX      64
#define FULLNAME_MAX   255

#define BAK_SUFFIX "bak"
#define MAX_LOAD_FAIL 5

#define STO_NORMAL    0
#define STO_BACKUP    1

typedef struct __attribute__((packed)) _tpl {
    uint32_t id;
    uint32_t len;
    uint8_t name[FNAME_MAX];
    int32_t checksum;
    uint32_t failcount;
} T_TPL_MAP;

typedef struct __attribute__((packed)) _tpls {
    int64_t db_id;
    T_TPL_MAP tpl[TPL_MAX_ST];
    uint32_t num;
    int32_t init_once;
    int32_t checksum;
} T_TPL_SET;

static uint8_t m_str_tpl_path[PATH_MAX] = {0};

static T_TPL_SET tpls;
static uint32_t lastId;

static uint32_t _fp_storage_get_next_fid(void);

#define TPL_FNAME ((const uint8_t *)"silead.tpl")

static store_if_t m_storage;
static int32_t m_inited = 0;

static int32_t _storage_register(void)
{
    int32_t ret = -1;

    if (!m_inited) {
        ret = silfp_storage_getif(&m_storage);
        if (ret >= 0) {
            m_inited = 1;
        }
    }

    if (!m_inited) {
        memset(&m_storage, 0, sizeof(m_storage));
        LOG_MSG_DEBUG("not implement storage");
    }

    return m_inited ? 0 : -1;
}

static int32_t _storage_check_dir(const uint8_t *path)
{
    if (m_storage.check_dir != NULL) {
        if (m_storage.check_dir(path) < 0) {
            return -SL_ERROR_STO_OP_FAILED;
        }
    }
    return SL_SUCCESS;
}

static int32_t _storage_read_file(const uint8_t *fname, uint8_t *buf, uint32_t size)
{
    int32_t ret;

    if (_storage_register() < 0) {
        return -SL_ERROR_OP_IMPL_INVALID;
    }

    ret = -SL_ERROR_STO_OP_FAILED;
    if (m_storage.read_file != NULL) {
        ret = m_storage.read_file(fname, buf, size);
    } else {
        LOG_MSG_DEBUG("read_file not implement");
    }
    return ret;
}

static int32_t _storage_write_file(const uint8_t *fname, const uint8_t *buf, uint32_t len)
{
    int32_t ret;

    if (_storage_register() < 0) {
        return -SL_ERROR_OP_IMPL_INVALID;
    }

    ret = -SL_ERROR_STO_OP_FAILED;
    if (m_storage.write_file != NULL) {
        ret = m_storage.write_file(fname, buf, len);
    } else {
        LOG_MSG_DEBUG("write_file not implement");
    }

    return ret;
}

static int32_t _storage_remove_file(const uint8_t *fname)
{
    int32_t ret;

    if (_storage_register() < 0) {
        return -SL_ERROR_OP_IMPL_INVALID;
    }

    ret = -SL_ERROR_STO_OP_FAILED;
    if (m_storage.remove_file != NULL) {
        ret = m_storage.remove_file(fname);
    } else {
        LOG_MSG_DEBUG("remove_file not implement");
    }

    return ret;
}

uint32_t fp_storage_normal_style(void)
{
    if (_storage_register() < 0) {
        return 1;
    }

    if (m_storage.normal_style != NULL) {
        return m_storage.normal_style();
    }
    return 1;
}

static const uint8_t *_storage_get_tpl_path(void)
{
    if (m_str_tpl_path[0]) {
        return m_str_tpl_path;
    } else {
        return (const uint8_t *)"/data/silead/fp/";
    }
}

int32_t fp_storage_set_tpl_path(const uint8_t *path, const uint32_t len)
{
    uint32_t size = len;
    if (path != NULL && len > 0) {
        if (size >=  PATH_MAX - 1) {
            size = PATH_MAX - 2;
        }
        strncpy((char *)m_str_tpl_path, (char *)path, size);
        if (m_str_tpl_path[size - 1] != '/') {
            m_str_tpl_path[size++] = '/';
        }
        m_str_tpl_path[size] = 0;
    }

    LOG_MSG_VERBOSE("path = %s", m_str_tpl_path);

    return SL_SUCCESS;
}

static int32_t _storage_make_filename(const uint8_t *path, const uint8_t *name, uint8_t *fname, uint32_t size, const uint8_t bak)
{
    const uint8_t *p = path;
    uint32_t len;
    uint32_t name_len;

    if (name == NULL || fname == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (p == NULL) {
        p = _storage_get_tpl_path();
    }

    if (p == NULL) {
        LOG_MSG_DEBUG("get tpl path failed");
        return -SL_ERROR_STO_OP_FAILED;
    }

    len = strlen((const char *)p);
    name_len = strlen((const char *)name);
    if (len + name_len >= size) {
        LOG_MSG_ERROR("fname buf is too small");
        return -SL_ERROR_STO_OP_FAILED;
    }

    strncpy((char *)fname, (char *)p, len);
    strncpy((char *)fname + len, (char *)name, name_len);
    if (STO_BACKUP == bak) {
        memcpy(&fname[len+name_len-strlen(BAK_SUFFIX)],BAK_SUFFIX,strlen(BAK_SUFFIX));
    }
    fname[len + name_len] = 0;

    //LOG_MSG_VERBOSE("fname = %s", fname);

    return SL_SUCCESS;
}

static int32_t _storage_calc_checksum(const uint8_t *in, const uint32_t len, int32_t *checksum)
{
    uint32_t i;
    unsigned long int sum = 0;
    uint8_t *p;

    if (in == NULL || checksum == NULL || len == 0) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    p = (uint8_t *)in;
    for (i = 0; i < len; i++) {
        sum += *p++;
    }

    while (sum >> 16) {
        sum = (sum >> 16) + (sum & 0xFFFF);
    }

    *checksum = ~sum;

    return SL_SUCCESS;
}

static int32_t _storage_save_sets(const uint8_t *path, const uint8_t *name, T_TPL_SET *sets, const uint8_t bak)
{
    int32_t ret = SL_SUCCESS;
    uint8_t fname[FULLNAME_MAX];
    int32_t checksum;

    if (path == NULL || name == NULL || sets == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _storage_check_dir(path);
    if (ret < 0) {
        return ret;
    }

    ret = _storage_make_filename(path, name, fname, sizeof(fname), bak);
    if (ret < 0) {
        return ret;
    }

    _storage_calc_checksum((uint8_t *)sets, sizeof(T_TPL_SET) - sizeof(int32_t), &checksum);
    sets->checksum = checksum;

    ret = _storage_write_file(fname, (uint8_t *)sets, sizeof(T_TPL_SET));
    if (ret != sizeof(T_TPL_SET)) {
        LOG_MSG_DEBUG("write failed (%d)", ret);
        return -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

static int32_t _storage_init()
{
    T_TPL_SET *ptpls = &tpls;
    uint8_t fname[FULLNAME_MAX];
    const uint8_t *path = NULL;
    int32_t ret, i;
    int32_t checksum;
    uint8_t bak = STO_NORMAL;

    /*if (ptpls->init_once) {
        return ptpls->num;
    }*/

    path = _storage_get_tpl_path();
    if (path == NULL) {
        LOG_MSG_DEBUG("get tpl path failed");
        return -SL_ERROR_STO_OP_FAILED;
    }

    ret = _storage_make_filename(path, TPL_FNAME, fname, sizeof(fname), bak);
    if (ret < 0) {
        return ret;
    }

    lastId = 0;

    do {
        memset(ptpls, 0, sizeof(T_TPL_SET));

        ret = _storage_read_file(fname, (uint8_t *)ptpls, sizeof(T_TPL_SET));
        if (ret == sizeof(T_TPL_SET)) {
            _storage_calc_checksum((uint8_t *)ptpls, sizeof(T_TPL_SET) - sizeof(int32_t), &checksum);
        }
        if (ret != sizeof(T_TPL_SET) || checksum != ptpls->checksum) {
            LOG_MSG_DEBUG("fingerprint data has been tampered with (%d), try backup", ret);
            bak = STO_BACKUP;
            ret = _storage_make_filename(path, TPL_FNAME, fname, sizeof(fname), bak);
            if (ret >= 0) {
                memset(ptpls, 0, sizeof(T_TPL_SET));
                ret = _storage_read_file(fname, (uint8_t *)ptpls, sizeof(T_TPL_SET));
                if (ret == sizeof(T_TPL_SET)) {
                    _storage_calc_checksum((uint8_t *)ptpls, sizeof(T_TPL_SET) - sizeof(int32_t), &checksum);
                }
                if (ret != sizeof(T_TPL_SET) || checksum != ptpls->checksum) {
                    //LOG_MSG_DEBUG("fingerprint data has been tampered with (%d), reset", ret);
                } else {
                    LOG_MSG_DEBUG("restore fingerprint data with backup");
                    _storage_save_sets(path, TPL_FNAME, ptpls, STO_NORMAL);
                }
            }
        }
        if (ret != sizeof(T_TPL_SET) || checksum != ptpls->checksum) {
            LOG_MSG_DEBUG("fingerprint data has been tampered with (%d), reset", ret);
            memset(ptpls, 0, sizeof(T_TPL_SET));
            ptpls->init_once = 1;
            ptpls->db_id = fp_util_get_64bit_rand();
            ret = _storage_save_sets(path, TPL_FNAME, ptpls, STO_BACKUP);
            ret = _storage_save_sets(path, TPL_FNAME, ptpls, STO_NORMAL);
            ret = 0;
            break;
        } else {
            ptpls->num = 0;
            for (i = 0; i < TPL_MAX_ST; i++) {
                if (ID_VALID(ptpls->tpl[i].id)) {
                    ptpls->num++;
                    if (lastId < ptpls->tpl[i].id) {
                        lastId = ptpls->tpl[i].id;
                    }
                }
            }
            ptpls->init_once = 1;
            ret = ptpls->num;
        }
    } while (0);

    return ret;
}

int32_t fp_storage_get_idlist(uint32_t *idlist, uint32_t count, const int32_t force)
{
    int32_t i, ret;
    T_TPL_SET *ptpls = &tpls;
    uint32_t num = 0;

    if (idlist == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    if (!ptpls->init_once || force) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    for (i = 0; i < TPL_MAX_ST && num <= count; i ++) {
        if (ID_VALID(ptpls->tpl[i].id)) {
            *idlist++ = ptpls->tpl[i].id;
            num++;
        }
    }

    return (int32_t)num;
}

static int32_t _storage_save(const uint8_t *name, const uint8_t *buf, uint32_t len, const uint8_t bak)
{
    uint8_t fname[FULLNAME_MAX];
    int32_t ret;

    if (buf == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _storage_make_filename(NULL, name, fname, sizeof(fname), bak);
    if (ret < 0) {
        return ret;
    }

    ret = _storage_write_file(fname, buf, len);
    if (ret < 0) {
        LOG_MSG_ERROR("write fail");
        return -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

static int32_t _storage_remove(const uint8_t *name, const uint8_t bak)
{
    uint8_t fname[FULLNAME_MAX];
    int32_t ret;

    if (name == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _storage_make_filename(NULL, name, fname, sizeof(fname), bak);
    if (ret < 0) {
        return ret;
    }

    ret = _storage_remove_file(fname);
    if (ret < 0) {
        LOG_MSG_ERROR("remove failed (%s)", fname);
    }

    ret = SL_SUCCESS;

    return ret;
}

int32_t fp_storage_update(const uint32_t id, const uint8_t *buf, const uint32_t len)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t i, ret = SL_SUCCESS;
    int32_t checksum = 0;
    uint8_t bak = STO_NORMAL;

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    for (i = 0; i < TPL_MAX_ST; i ++) {
        if (ptpls->tpl[i].id == id) {
            _storage_calc_checksum(buf, len, &checksum);
            if (checksum != ptpls->tpl[i].checksum) {
                bak = STO_BACKUP;
                ret = _storage_save(ptpls->tpl[i].name, buf, len, bak);
                if (ret >= 0) { // Checksum updated, need save it.
                    ptpls->tpl[i].len = ret;
                    ptpls->tpl[i].checksum = checksum;
                    if (_storage_save_sets(_storage_get_tpl_path(), TPL_FNAME, ptpls, bak) < 0) {
                        LOG_MSG_DEBUG("Oooops, save tpl config fail. but tpl data is saved.");
                        break;
                    }
                }

                if ((ret >= 0) && (STO_BACKUP == bak)) {
                    bak = STO_NORMAL;
                    ret = _storage_save_sets(_storage_get_tpl_path(), TPL_FNAME, ptpls, bak);
                    if (ret >= 0) { // Checksum updated, need save it.
                        if (_storage_save(ptpls->tpl[i].name, buf, len, bak) < 0) {
                            LOG_MSG_DEBUG("Oooops, save tpl data fail. but tpl config is saved.");
                        }
                    }
                }
            }
            break;
        }
    }

    if (i >= TPL_MAX_ST) {
        LOG_MSG_DEBUG("update failed, not find the tpl");
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

int32_t fp_storage_save(const uint8_t *buf, const uint32_t len, uint32_t *fid)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t i = 0, ret = SL_SUCCESS;
    uint32_t id;
    int32_t max_fail_index = -1;
    uint32_t max_fail = 0;
    int32_t checksum;
    uint8_t bak = STO_NORMAL;

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    id = _fp_storage_get_next_fid();

    for (i = 0; i < TPL_MAX_ST; i ++) {
        if (ptpls->tpl[i].id == id) {
            LOG_MSG_DEBUG("save failed, already exist");
            ret = -SL_ERROR_STO_OP_FAILED;
        }
    }

    if (ret >= 0) {
        for (i = 0; i < TPL_MAX_ST; i ++) {
            if (!ID_VALID(ptpls->tpl[i].id)) {
                break;
            } else {
                if (ptpls->tpl[i].failcount > max_fail) {
                    max_fail = ptpls->tpl[i].failcount;
                    max_fail_index = i;
                }
            }
        }

        if (i >= TPL_MAX_ST) {
            if (max_fail_index >= 0 && max_fail_index < TPL_MAX_ST) {
                i = max_fail_index;
            }
        }

        if (i >= 0 && i < TPL_MAX_ST) {
            bak = STO_BACKUP;
            if (max_fail_index == i) {
                ptpls->num--;
                _storage_remove(ptpls->tpl[i].name, bak);
            }

            memset(&ptpls->tpl[i], 0, sizeof(T_TPL_MAP));
            snprintf((char *)ptpls->tpl[i].name, FNAME_MAX, "f%08x.dat", id);//??
            ret = _storage_save((const uint8_t *)ptpls->tpl[i].name, buf, len, bak);
            if (ret >= 0) {
                ptpls->tpl[i].id = id;
                ptpls->tpl[i].len = ret;
                _storage_calc_checksum(buf, len, &checksum);
                ptpls->tpl[i].checksum = checksum;

                ptpls->num ++;
                ret = _storage_save_sets(_storage_get_tpl_path(), TPL_FNAME, ptpls, bak);
                if (ret < 0) {
                    _storage_remove(ptpls->tpl[i].name, bak);
                    memset(&ptpls->tpl[i], 0, sizeof(T_TPL_MAP));
                    ptpls->num --;
                } else {
                    ret = 0;
                    if (fid != NULL) {
                        *fid = id;
                    }
                }
            }

            if ((ret >= 0) && (STO_BACKUP == bak)) {
                bak = STO_NORMAL;
                if (max_fail_index == i) {
                    _storage_remove(ptpls->tpl[i].name, bak);
                }

                /* Save config 1st, otherwise if tpl saved, config not, tpl will be discarded when reload. */
                ret = _storage_save_sets(_storage_get_tpl_path(), TPL_FNAME, ptpls, bak);
                if (ret >= 0) {
                    ret = _storage_save((const uint8_t *)ptpls->tpl[i].name, buf, len, bak);
                }
            }
        }
    }

    if (i >= TPL_MAX_ST) {
        LOG_MSG_DEBUG("save failed, tpl full");
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

static int32_t _storage_load(const uint8_t *name, uint8_t *buf, uint32_t len, const uint8_t bak)
{
    uint8_t fname[FULLNAME_MAX];
    int32_t ret;

    if (buf == NULL || len == 0) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _storage_make_filename(NULL, name, fname, sizeof(fname), bak);
    if (ret < 0) {
        return ret;
    }

    ret = _storage_read_file(fname, buf, len);
    if (ret < 0) {
        LOG_MSG_ERROR("read fail");
        return -SL_ERROR_STO_OP_FAILED;
    }

    return (int32_t)len;
}

int32_t fp_storage_load(const uint32_t id, uint8_t *buf, const uint32_t len)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t i, ret = SL_SUCCESS;
    uint32_t size = len;
    uint32_t oldfailcount;
    int32_t checksum;
    uint8_t bak = STO_NORMAL, tpl_del = 0;
    uint8_t fname[FNAME_MAX];

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    for (i = 0; i < TPL_MAX_ST; i ++) {
        if (ptpls->tpl[i].id == id) {
            if (size > ptpls->tpl[i].len) {
                size = ptpls->tpl[i].len;
            }
            oldfailcount = ptpls->tpl[i].failcount;
            bak = STO_NORMAL;
            ret = _storage_load((const uint8_t *)ptpls->tpl[i].name, buf, size, bak);
            if (ret > 0) {
                _storage_calc_checksum(buf, ret, &checksum);
                if (ptpls->tpl[i].checksum != checksum) {
                    //ptpls->tpl[i].failcount++;
                    LOG_MSG_DEBUG("load failed, tpl invalid");
                    ret = -SL_ERROR_STO_OP_FAILED;
                } else {
                    ptpls->tpl[i].failcount = 0;
                }
            }

            if (ret <= 0) {
                bak = STO_BACKUP;
                ret = _storage_load((const uint8_t *)ptpls->tpl[i].name, buf, size, bak);
                if (ret > 0) {
                    _storage_calc_checksum(buf, ret, &checksum);
                    if (ptpls->tpl[i].checksum != checksum) {
                        ptpls->tpl[i].failcount++;
                        LOG_MSG_DEBUG("load failed, tpl invalid");
                        ret = -SL_ERROR_STO_OP_FAILED;
                    } else {
                        ptpls->tpl[i].failcount = 0;
                        LOG_MSG_DEBUG("restore tpl with backup");
                        _storage_save((const uint8_t *)ptpls->tpl[i].name, buf, ret, STO_NORMAL);
                    }
                } else {
                    ptpls->tpl[i].failcount++;
                }
            }

            if (oldfailcount != ptpls->tpl[i].failcount) {
                bak = STO_BACKUP;
                if (ptpls->tpl[i].failcount >= MAX_LOAD_FAIL) {
                    tpl_del = 1;
                    T_TPL_MAP tpl = ptpls->tpl[i];
                    memcpy(fname, tpl.name, sizeof(fname));
                    memset(&ptpls->tpl[i], 0, sizeof(T_TPL_MAP));
                    ptpls->num--;
                    _storage_remove((const uint8_t *)fname, bak);
                    LOG_MSG_DEBUG("max load error, reset the tpl");
                }
                if(_storage_save_sets(_storage_get_tpl_path(), TPL_FNAME, ptpls, bak) < 0) {
                    LOG_MSG_DEBUG("Ooops, how can I do!!!");
                }

                if (STO_BACKUP == bak) {
                    bak = STO_NORMAL;
                    if (tpl_del) {
                        _storage_remove((const uint8_t *)fname, bak);
                    }
                    if(_storage_save_sets(_storage_get_tpl_path(), TPL_FNAME, ptpls, bak) < 0) {
                        LOG_MSG_DEBUG("Ooops, how can I do!!!");
                    }
                }
            }
            break;
        }
    }

    if (i >= TPL_MAX_ST) {
        LOG_MSG_ERROR("load failed, not find the tpl");
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

int32_t fp_storage_remove(const uint32_t id)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t i, ret = SL_SUCCESS;
    uint8_t bak = STO_NORMAL;

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    for (i = 0; i < TPL_MAX_ST; i ++) {
        if (ptpls->tpl[i].id == id) {
            bak = STO_BACKUP;
            T_TPL_MAP tpl = ptpls->tpl[i];
            memset(&ptpls->tpl[i], 0, sizeof(T_TPL_MAP));
            ptpls->num--;
            ret = _storage_save_sets(_storage_get_tpl_path(), (uint8_t *)TPL_FNAME, ptpls, bak);
            if (ret >= 0) {
                ret = _storage_remove(tpl.name, bak);
                ret = 0;
            } else {
                ptpls->tpl[i] = tpl;
                ptpls->num ++;
                ret = -SL_ERROR_STO_OP_FAILED;
            }

            if ((ret >= 0) && (STO_BACKUP == bak)) {
                bak = STO_NORMAL;
                ret = _storage_save_sets(_storage_get_tpl_path(), (uint8_t *)TPL_FNAME, ptpls, bak);
                if (ret >= 0) {
                    ret = _storage_remove(tpl.name, bak);
                }
            }
            break;
        }
    }

    if (i >= TPL_MAX_ST) {
        LOG_MSG_DEBUG("remove failed, not find the tpl");
        //ret = -SL_ERROR_STO_OP_FAILED;
        ret = 0; // if the tpl not exist, then report success
    }

    return ret;
}

int32_t fp_storage_init(void)
{
    if (_storage_register() < 0) {
        return -SL_ERROR_OP_IMPL_INVALID;
    }

    return SL_SUCCESS;
}

int32_t fp_storage_deinit(void)
{
    LOG_MSG_VERBOSE("storage release");
    return 0;
}

static uint32_t _fp_storage_get_next_fid(void)
{
    T_TPL_SET *ptpls = &tpls;
    int64_t random = 0;
    uint32_t *pd = (uint32_t *)&random;
    uint32_t newid = 0;
    int32_t i;
    int32_t found = 0;

    do {
        random = fp_util_get_64bit_rand();
        newid = (pd[0] ^ pd[1]);

        newid = newid | TPLID_S;
        LOG_MSG_VERBOSE("gen newid: 0x%08x", newid);

        found = 0;
        for (i = 0; i < TPL_MAX_ST; i++) {
            if (ptpls->tpl[i].id == newid) {
                found = 1;
                break;
            }
        }
    } while (found != 0);

    return newid;
}

int32_t fp_storage_inc_fail_count(const uint32_t id)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t i, ret = SL_SUCCESS;
    uint8_t bak = STO_NORMAL, tpl_del = 0;
    uint8_t fname[FNAME_MAX];

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    for (i = 0; i < TPL_MAX_ST; i ++) {
        if (ptpls->tpl[i].id == id) {
            bak = STO_NORMAL;
            ptpls->tpl[i].failcount++;
            if (ptpls->tpl[i].failcount >= MAX_LOAD_FAIL) {
                tpl_del = 1;
                T_TPL_MAP tpl = ptpls->tpl[i];
                memcpy(fname, tpl.name, sizeof(fname));
                memset(&ptpls->tpl[i], 0, sizeof(T_TPL_MAP));
                ptpls->num--;
                _storage_remove(fname, bak);
                LOG_MSG_DEBUG("max load error, reset the tpl");
            }
            if(_storage_save_sets(_storage_get_tpl_path(), TPL_FNAME, ptpls, bak) < 0) {
                LOG_MSG_DEBUG("Ooops, how can I do!!!");
                break;
            }

            if ((ret >= 0) && (STO_BACKUP == bak)) {
                bak = STO_NORMAL;
                if (tpl_del) {
                    _storage_remove(fname, bak);
                }
                if(_storage_save_sets(_storage_get_tpl_path(), TPL_FNAME, ptpls, bak) < 0) {
                    LOG_MSG_DEBUG("Ooops, how can I do!!!");
                }
            }
            break;
        }
    }

    if (i >= TPL_MAX_ST) {
        LOG_MSG_DEBUG("load failed, not find the tpl");
        ret = -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

int64_t fp_storage_load_db_id(void)
{
    T_TPL_SET *ptpls = &tpls;
    int32_t ret = SL_SUCCESS;

    if (!ptpls->init_once) {
        ret = _storage_init();
        if (ret < 0) {
            return ret;
        }
    }

    return ptpls->db_id;
}

int32_t fp_storage_get_filename(const uint8_t *path, const uint8_t *name, uint8_t *fname, uint32_t size)
{
    return _storage_make_filename(path, name, fname, size, 0);
}

#ifdef SIL_CAL_STORE
static uint8_t m_str_cal_path[PATH_MAX] = {0};
static const uint8_t *_storage_get_cal_path(void)
{
    if (m_str_cal_path[0]) {
        return m_str_cal_path;
    } else {
        return (const uint8_t *)"/data/system/silead/";
    }
}

int32_t fp_storage_set_cal_path(const uint8_t *path, const uint32_t len)
{
    uint32_t size = len;
    if (path != NULL && len > 0) {
        if (size >=  PATH_MAX - 1) {
            size = PATH_MAX - 2;
        }
        strncpy((char *)m_str_cal_path, (char *)path, size);
        if (m_str_cal_path[size - 1] != '/') {
            m_str_cal_path[size++] = '/';
        }
        m_str_cal_path[size] = 0;
    }

    LOG_MSG_VERBOSE("path = %s", m_str_cal_path);

    return SL_SUCCESS;
}

int32_t fp_storage_load_cal(const char *name, uint8_t *buf, uint32_t len)
{
    uint8_t fname[FULLNAME_MAX];
    int32_t ret;

    if (name == NULL || buf == NULL || len == 0) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _storage_make_filename(_storage_get_cal_path(), name, fname, sizeof(fname), 0);
    if (ret < 0) {
        return ret;
    }

    ret = _storage_read_file(fname, buf, len);
    if (ret < 0) {
        LOG_MSG_ERROR("read fail");
        return -SL_ERROR_STO_OP_FAILED;
    }

    return (int32_t)len;
}

int32_t fp_storage_save_cal(const uint8_t *name, const uint8_t *buf, uint32_t len)
{
    uint8_t fname[FULLNAME_MAX];
    int32_t ret;

    if (buf == NULL || len == 0) {
        LOG_MSG_ERROR("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _storage_make_filename(_storage_get_cal_path(), name, fname, sizeof(fname), 0);
    if (ret < 0) {
        return ret;
    }

    ret = _storage_write_file(fname, buf, len);
    if (ret < 0) {
        LOG_MSG_ERROR("write fail");
        return -SL_ERROR_STO_OP_FAILED;
    }

    return ret;
}

int32_t fp_storage_remove_cal(const uint8_t *name)
{
    uint8_t fname[FULLNAME_MAX];
    int32_t ret;

    if (name == NULL) {
        LOG_MSG_DEBUG("param invalid");
        return -SL_ERROR_BAD_PARAMS;
    }

    ret = _storage_make_filename(_storage_get_cal_path(), name, fname, sizeof(fname), 0);
    if (ret < 0) {
        return ret;
    }

    ret = _storage_remove_file(fname);
    if (ret < 0) {
        LOG_MSG_ERROR("remove failed (%s)", fname);
    }

    ret = SL_SUCCESS;

    return ret;
}

#endif /*SIL_CAL_STORE */
