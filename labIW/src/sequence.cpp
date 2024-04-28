#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const float __my_pi = 3.14159265359;

struct Sequence
{
    public:
    int size;
    float* data;

    Sequence (int size) {
        this->size = size;
        this->data = new float[size];
        for (int i=0; i<size; i++) {
            this->data[i] = 0.0f;
        }
    }

    static Sequence* from_file (const char* filename) {
        FILE* source = fopen(filename, "r");
        if (source == NULL) {
            return NULL;
        }
        int size = 0;
        fscanf(source, "%d", &size);
        if (size <= 0) {
            fclose(source);
            return NULL;
        }
        Sequence* result = new Sequence(size);
        float n = 0.0f;
        for (int i=0; i<size && !feof(source); i++) {
            fscanf(source, "%f", &n);
            result->data[i] = n;
        }
        fclose(source);
        return result;
    }

    static Sequence* mock_sinusoid (int size, int period, double amp, double noise_amp) {
        Sequence* result = new Sequence(size);
        srand(time(NULL));
        for (int i=0; i<size; i++) {
            result->data[i] = amp * sin(__my_pi * double(i) / double(period)) +
                              noise_amp / double(rand()%100 + 1); 
        }
        return result;
    }

    Sequence* empty_autocor_result () {
        return new Sequence(this->size / 2 + 5);
    }

    int index_min (int start_index=0) {
        float val = this->data[start_index];
        int index = start_index;
        for (int i=start_index; i<this->size; i++) {
            if (this->data[i] < val) {
                val = this->data[i];
                index = i;
            }
        }
        return index;
    }

    int index_max (int start_index=0) {
        float val = this->data[start_index];
        int index = start_index;
        for (int i=start_index; i<this->size; i++) {
            if (this->data[i] > val) {
                val = this->data[i];
                index = i;
            }
        }
        return index;
    }

    float min (int start_index=0) {
        float val = this->data[start_index];
        for (int i=start_index; i<this->size; i++) {
            if (this->data[i] < val) {
                val = this->data[i];
            }
        }
        return val;
    }

    float max (int start_index=0) {
        float val = this->data[start_index];
        for (int i=start_index; i<this->size; i++) {
            if (this->data[i] > val) {
                val = this->data[i];
            }
        }
        return val;
    }
};
