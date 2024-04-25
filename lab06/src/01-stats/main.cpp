#include <stdio.h>
#include <ctype.h>
#include <wctype.h>
#include <wchar.h>
#include <string.h>
#include <locale.h>
#include <pthread.h>

typedef pthread_mutex_t mutex;
typedef wint_t wchar;

mutex shared_mutex;

struct numeric_file_stats {
    int count_total;
    int count_positive;
    int count_negative;
    double min;
    double max;
    double average;
    double sum;
};

struct text_file_stats {
    int count_word;
    int count_line;
    int count_letter;
    int count_digit;
    int count_punct;
};

// params: 1: source file pointer
// returns: numeric file stats
numeric_file_stats get_numeric_stats (FILE* source) {
    bool any = false;
    numeric_file_stats result;
    result.count_total = 0;
    result.count_positive = 0;
    result.count_negative = 0;
    double number = 0; 
    while (true) { 
        if (fscanf(source, "%lf", &number)<=0) {
            fgetc(source);
            if (feof(source)) break; 
            continue;
        }
        if (!any) {
            result.min = number;
            result.max = number;
            any = true;
        }
        result.count_total += 1;
        if (number >= 0) result.count_positive += 1;
        else result.count_negative += 1;
        if (result.min > number) result.min = number;
        if (result.max < number) result.max = number;
        result.sum += number;
    }
    result.average = result.sum / (double)result.count_total;
    return result;
}

// params: 1: source file pointer
// returns: text file stats
text_file_stats get_text_stats (FILE* source) {
    text_file_stats result;
    result.count_word = 0;
    result.count_letter = 0;
    result.count_punct = 0;
    result.count_line = 1;
    bool word = false; 
    int buffer_size = 256;
    char* buffer = new char[buffer_size+1];
    mbstate_t ps;
    memset(&ps, 0, sizeof(ps)); // init mbstate
    while (true) {
        int real_size = fread(buffer, 1, buffer_size, source);
        if (real_size <= 0) break;
        buffer[real_size+1] = '\0';
        int pos = 0;
        wchar_t c1;
        while (true) {
            int b = mbrtowc(&c1, buffer + pos, 8, &ps); // convert next utf8 byte seq to wide char
            wchar c = c1;
            if (b <= 0) break;
            pos += b;
            if (iswalnum(c)) {
                if (iswalpha(c)) {
                    result.count_letter += 1;
                }
                if (iswdigit(c)) {
                    result.count_digit += 1;
                }
                if (!word) {
                    word = true;
                    result.count_word += 1;
                }
            }
            else {
                if (word) {
                    word = false;
                }
                if (iswpunct(c)) {
                    result.count_punct += 1;
                }
                if (c == (wchar)'\n') {
                    result.count_line += 1;
                }
            }
        }
    }
    delete[] buffer;
    return result;
}

// handle numeric file
// params: 1: filename
// returns: void
// prints: file stats
void handle_numeric_file (char* filename) {
    FILE* source = fopen(filename, "r");
    if (source == NULL) {
        printf("Can not open %s\n", filename);
        return;
    }
    numeric_file_stats num_stats = get_numeric_stats(source);
    fclose(source); 
    pthread_mutex_lock(&shared_mutex); // to prevent stdout from scuffing
    printf("Numeric stats for %s:\n", 
        filename);
    printf("- %d total\n- %d positive\n- %d negative\n", 
        num_stats.count_total, 
        num_stats.count_positive, 
        num_stats.count_negative);
    printf("- sum: %.3lf\n- average: %.3lf\n", 
        num_stats.sum, 
        num_stats.average);
    printf("- min: %.3lf\n- max: %.3lf\n", 
        num_stats.min, 
        num_stats.max);
    printf("\n");
    pthread_mutex_unlock(&shared_mutex);
}

// handle text file
// params: 1: filename
// returns: void
// prints: file stats
void handle_text_file (char* filename) {
    FILE* source = fopen(filename, "r");
    if (source == NULL) {
        printf("Can not open %s\n", filename);
        return;
    }
    text_file_stats text_stats = get_text_stats(source);
    fclose(source); 
    pthread_mutex_lock(&shared_mutex); // to prevent stdout from scuffing
    printf("Text stats for %s:\n", 
        filename);
    printf("- %d words\n- %d lines\n", 
        text_stats.count_word, 
        text_stats.count_line);
    printf("- %d letters\n- %d digits\n- %d punctuations\n", 
        text_stats.count_letter, 
        text_stats.count_digit,
        text_stats.count_punct);
    printf("\n");
    pthread_mutex_unlock(&shared_mutex);
}

// thread entry point
// params: 1: type=filename
// returns: null
void* handle_file (void* arg) {
    char* type_and_filename = (char*) arg;
    if (strncmp(type_and_filename, "num=", 4) == 0) {
        handle_numeric_file(type_and_filename + 4);
    }
    else if (strncmp(type_and_filename, "text=", 5) == 0) {
        handle_text_file(type_and_filename + 5);
    }
    else {
        pthread_mutex_lock(&shared_mutex); // to prevent stdout from scuffing
        printf("Can not handle %s\n", type_and_filename);
        pthread_mutex_unlock(&shared_mutex);
    }
    return NULL;
}

int _01_stats_main (int argc, char ** argv) {
    setlocale(LC_ALL, "en_US.UTF-8");
    if (argc < 2) {
        printf("usage: %s <filename>*\n", argv[0]);
        return 1;
    }
    argc -= 1;
    argv += 1;
    int num_threads = 4;
    pthread_mutex_init(&shared_mutex, NULL);
    // handle all filenames in batches of num_threads size
    while (argc > 0) {
        pthread_t* threads = new pthread_t[num_threads];
        int real_num_threads = 0;
        for (int i=0; i<num_threads && argc>0; i+=1) {
            // create and start thread
            pthread_create(threads + i, NULL, handle_file, argv[0]);
            argc -= 1;
            argv += 1;
            real_num_threads += 1;
        }
        for (int i=0; i<real_num_threads; i+=1) {
            if (threads[i]!=0) {
                // await each of started threads
                pthread_join(threads[i], NULL);
            }
        }
        delete[] threads;
    }
    pthread_mutex_destroy(&shared_mutex);
    return 0;
}
