#ifndef ITQ_ATLAS_H
#define ITQ_ATLAS_H

#include "mat.h"
#include "lsh.h"
#include <vector>

namespace lsh {
class ITQ : public LSH {
    public:
        ITQ() {}
        ITQ(size_t n_bit, size_t n_dim, size_t n_table)
            : n_bit(n_bit), n_dim(n_dim), n_table(n_table) {}
        virtual ~ITQ() {
            for(size_t i = 0; i < p_pca_vec.size(); ++ i) {
                delete [] p_pca_vec[i];
                delete [] p_r_vec[i];
            }
        }
        bool LoadModel(const char *model_file) {
            float *p_tmp = NULL;
            pca_vec.reserve(n_table);
            r_vec.reserve(n_table);
            for(size_t i = 0; i < n_table; ++ i) {
                p_tmp = new float[n_dim * n_bit];
                p_pca_vec.push_back(p_tmp);
                p_tmp = NULL;
                p_tmp = new float[n_bit * n_bit];
                p_r_vec.push_back(p_tmp);
            }
            FILE *fp = fopen(model_file, "rb");
            // TODO: check model file valid
            if(fp) {
                for(size_t i = 0; i < n_table; ++ i) {
                    fread(p_pca_vec[i], 1, sizeof(float) * n_dim * n_bit, fp);
                    fread(p_r_vec[i], 1, sizeof(float) * n_bit * n_bit, fp);
                    pca_vec[i].Set(p_pca_vec[i], n_dim, n_bit);
                    r_vec[i].Set(p_r_vec[i], n_bit, n_bit);
                }
                return true;
            }
            fclose(fp);
            return false;
        }

        inline const std::vector<std::vector<bool> > Hash(const mat::Mat &input, mat::Mat &pca_rs, mat::Mat &relax) {
            std::vector<std::vector<bool> > rs;
            for(size_t i = 0; i < n_table; ++ i) {
                std::vector<bool> hash_rs(n_bit);
                pca_rs.Reset();
                relax.Reset();
                // pca_rs = gemm(input, pca_vec[i]);
                // relax = gemm(pca_rs, r_vec[i]);
                sgemm(input, pca_vec[i], pca_rs);
                sgemm(pca_rs, r_vec[i], relax);
                for(size_t i = 0; i < relax.n_col; ++ i) {
                    hash_rs[i] = relax.dptr[i] > 0 ? true : false;
                }
                rs.push_back(hash_rs);
            }
            return rs;
        }

    private:
        size_t n_bit;
        size_t n_dim;
        size_t n_table;
        std::vector<mat::Mat> pca_vec;
        std::vector<float*> p_pca_vec;
        std::vector<mat::Mat> r_vec;
        std::vector<float*> p_r_vec;
        ITQ(const ITQ &other);
        ITQ& operator=(const ITQ &other);
};
}
#endif /*ITQ_ATLAS_H*/
