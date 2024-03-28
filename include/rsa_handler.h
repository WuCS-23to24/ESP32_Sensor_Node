#ifndef F04EB51A_DCC5_4C00_BF78_FFB264E40E8E
#define F04EB51A_DCC5_4C00_BF78_FFB264E40E8E

#include "mbedtls/bignum.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/esp_config.h"
#include "mbedtls/platform.h"
#include "mbedtls/rsa.h"

#include <cstdio>
#include <cstring>

#include <cmath>

enum RSA_CODES
{
    PUB_KEY_IMPORT_SUCCESS = 1,
    PUB_KEY_EXPORT_SUCCESS = 2,
    DATA_ENCRYPTION_SUCCESS = 3,
    DATA_DECRYPTION_SUCCESS = 4,
    COULD_NOT_IMPORT_REMOTE_RSA_MODULUS_STRING = -1,
    COULD_NOT_IMPORT_REMOTE_RSA_PUBLIC_EXPONENT_STRING = -2,
    COULD_NOT_IMPORT_MPI_CONTEXTS_INTO_RSA_CONTEXT = -3,
    COULD_NOT_EXPORT_RSA_KEY_INTO_MPI_CONTEXT = -4,
    INPUT_BUFFER_IS_SMALLER_THAN_CTX_LENGTH = -5,
    COULD_NOT_ENCRYPT_DATA = -6,
    COULD_NOT_DECRYPT_DATA = -7
};

struct rsa_public_key
{
    char *rsa_modulus;
    char *rsa_public_exponent;

    int size_rsa_modulus;
    int size_rsa_public_exponent;
};

class RSA
{
  public:
    RSA()
    {
        int ret = 0;

        /*
         * Initialize all contexts
         */
        mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_rsa_init(&local_rsa, MBEDTLS_RSA_PKCS_V15, 0);
        mbedtls_mpi_init(&rsa_modulus);
        mbedtls_mpi_init(&rsa_public_exponent);
        mbedtls_mpi_init(&remote_rsa_modulus);
        mbedtls_mpi_init(&remote_rsa_public_exponent);
        mbedtls_entropy_init(&entropy);

        /*
         * Seed the CTR_DRBG context
         */
        if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers,
                                         pers_len)) != 0)
        {
            constructor_return_value = ret;

            mbedtls_mpi_free(&rsa_modulus);
            mbedtls_mpi_free(&rsa_public_exponent);
            mbedtls_mpi_free(&remote_rsa_modulus);
            mbedtls_mpi_free(&remote_rsa_public_exponent);
            mbedtls_rsa_free(&local_rsa);
            mbedtls_ctr_drbg_free(&ctr_drbg);
            mbedtls_entropy_free(&entropy);
        }

        /*
         * Generating RSA key
         */
        if ((ret = mbedtls_rsa_gen_key(&local_rsa, mbedtls_ctr_drbg_random, &ctr_drbg, key_size, exponent)) != 0)
        {
            constructor_return_value = ret;

            mbedtls_mpi_free(&rsa_modulus);
            mbedtls_mpi_free(&rsa_public_exponent);
            mbedtls_mpi_free(&remote_rsa_modulus);
            mbedtls_mpi_free(&remote_rsa_public_exponent);
            mbedtls_rsa_free(&local_rsa);
            mbedtls_ctr_drbg_free(&ctr_drbg);
            mbedtls_entropy_free(&entropy);
        }
    }

    int import_remote_rsa_public_key(char *remote_rsa_modulus_str, char *remote_rsa_public_exponent_str,
                                     int remote_rsa_modulus_len, int remote_rsa_public_exponent_len)
    {
        int ret = 0;

        if ((ret = mbedtls_mpi_read_string(&remote_rsa_modulus, default_radix, remote_rsa_modulus_str)) != 0)
        {
            return COULD_NOT_IMPORT_REMOTE_RSA_MODULUS_STRING;
        }

        if ((ret = mbedtls_mpi_read_string(&remote_rsa_public_exponent, default_radix,
                                           remote_rsa_public_exponent_str)) != 0)
        {
            return COULD_NOT_IMPORT_REMOTE_RSA_PUBLIC_EXPONENT_STRING;
        }

        if ((ret = mbedtls_rsa_import(&remote_rsa, &remote_rsa_modulus, NULL, NULL, NULL,
                                      &remote_rsa_public_exponent)) != 1)
        {
            return COULD_NOT_IMPORT_MPI_CONTEXTS_INTO_RSA_CONTEXT;
        }

        return PUB_KEY_IMPORT_SUCCESS;
    }

    int export_local_rsa_public_key(rsa_public_key *export_key)
    {
        unsigned int written_bytes = 0;
        int ret = 0;

        malloc_mpi_output_buffer(&rsa_modulus, 16, &export_key->rsa_modulus, &export_key->size_rsa_modulus);
        malloc_mpi_output_buffer(&rsa_public_exponent, 16, &export_key->rsa_public_exponent,
                                 &export_key->size_rsa_public_exponent);

        if ((ret = mbedtls_mpi_write_string(&rsa_modulus, 16, export_key->rsa_modulus, export_key->size_rsa_modulus,
                                            &written_bytes)) != 0 ||
            (ret = mbedtls_mpi_write_string(&rsa_public_exponent, 16, export_key->rsa_public_exponent,
                                            export_key->size_rsa_public_exponent, &written_bytes)) != 0)
        {
            return COULD_NOT_EXPORT_RSA_KEY_INTO_MPI_CONTEXT;
        }

        return PUB_KEY_EXPORT_SUCCESS;
    }

    /*
     * output_data gets allocated inside of encrypt_data, any data allocated to output_data prior will become
     * unreachable.
     */
    int encrypt_data(const char *input_data, size_t input_len, char **output_data, size_t *output_len)
    {
        if (input_len != remote_rsa.len)
        {
            return INPUT_BUFFER_IS_SMALLER_THAN_CTX_LENGTH;
        }

        *(unsigned char **)output_data = (unsigned char *)malloc(remote_rsa.len * sizeof(unsigned char));

        if (mbedtls_rsa_public(&remote_rsa, (unsigned char *)input_data, *(unsigned char **)output_data) != 0)
        {
            return COULD_NOT_ENCRYPT_DATA;
        }

        return DATA_ENCRYPTION_SUCCESS;
    }

    /*
     * output_data gets allocated inside of encrypt_data, any data allocated to output_data prior will become
     * unreachable.
     */
    int decrypt_data(const char *input_data, size_t input_len, char **output_data, size_t *output_len)
    {
        if (input_len != remote_rsa.len)
        {
            return INPUT_BUFFER_IS_SMALLER_THAN_CTX_LENGTH;
        }

        *(unsigned char **)output_data = (unsigned char *)malloc(remote_rsa.len * sizeof(unsigned char));

        if (mbedtls_rsa_private(&local_rsa, mbedtls_ctr_drbg_random, &ctr_drbg, (unsigned char *)input_data,
                                *(unsigned char **)output_data) != 0)
        {
            return COULD_NOT_DECRYPT_DATA;
        }

        return DATA_DECRYPTION_SUCCESS;
    }

  private:
    int constructor_return_value = 0;

    const char *pers = "rsa_genkey";
    const int pers_len = 11;
    const int default_radix = 16;
    const int key_size = 2048;
    const int exponent = std::pow(2, 60) + 1;

    mbedtls_rsa_context local_rsa;
    mbedtls_rsa_context remote_rsa;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_mpi rsa_modulus, rsa_public_exponent;
    mbedtls_mpi remote_rsa_modulus, remote_rsa_public_exponent;

    inline void malloc_mpi_output_buffer(mbedtls_mpi *X, int radix, char **buffer, int *len)
    {
        size_t n = mbedtls_mpi_bitlen(X);

        if (radix >= 4)
            n >>= 1;

        if (radix >= 16)
            n >>= 1;

        n += 1;
        n += 1;
        n += 1;
        n += (n & 1);

        *buffer = (char *)malloc(n * sizeof(char));
        *len = n;
    }
};

#endif /* F04EB51A_DCC5_4C00_BF78_FFB264E40E8E */
