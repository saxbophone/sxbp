/*
 * This source file forms sxbp, a command-line program which generates images of
 * experimental 2D spiral-like shapes in PBM format, based on input binary data.
 *
 * It uses libsaxbospiral to achieve this, a library available under the same
 * licensing terms as this program.
 *
 *
 *
 * Copyright (C) 2016, Joshua Saxby joshua.a.saxby+TNOPLuc8vM==@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License (version 3),
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <argtable2.h>
#include <saxbospiral-0.21/saxbospiral.h>
#include <saxbospiral-0.21/initialise.h>
#include <saxbospiral-0.21/solve.h>
#include <saxbospiral-0.21/serialise.h>
#include <saxbospiral-0.21/render.h>
#include <saxbospiral-0.21/render_backends/backend_pbm.h>
#include <saxbospiral-0.21/render_backends/backend_png.h>


#ifdef __cplusplus
extern "C"{
#endif

// returns size of file associated with given file handle
static size_t get_file_size(FILE* file_handle) {
    // seek to end
    fseek(file_handle, 0L, SEEK_END);
    // get size
    size_t file_size = ftell(file_handle);
    // seek to start again
    fseek(file_handle, 0L, SEEK_SET);
    return file_size;
}

/*
 * given an open file handle and a buffer, read the file contents into buffer
 * returns true on success and false on failure.
 */
static bool file_to_buffer(FILE* file_handle, sxbp_buffer_t* buffer) {
    size_t file_size = get_file_size(file_handle);
    // allocate/re-allocate buffer memory
    if(buffer->bytes == NULL) {
        buffer->bytes = calloc(1, file_size);
    } else {
        buffer->bytes = realloc(buffer->bytes, file_size);
    }
    if(buffer->bytes == NULL) {
        // couldn't allocate enough memory!
        return false;
    }
    buffer->size = file_size;
    // read in file data to buffer
    size_t bytes_read = fread(buffer->bytes, 1, file_size, file_handle);
    // check amount read was the size of file
    if(bytes_read != file_size) {
        // free memory
        free(buffer->bytes);
        return false;
    } else {
        return true;
    }
}

/*
 * given a C-style string and a buffer, reads the string into the buffer
 * returns true on success and false on failure.
 */
static bool string_to_buffer(const char* input_string, sxbp_buffer_t* buffer) {
    // get string length
    size_t input_length = strlen(input_string);
    // allocate buffer memory
    buffer->bytes = calloc(1, input_length);
    if(buffer->bytes == NULL) {
        // couldn't allocate memory!
        return false;
    }
    buffer->size = input_length;
    // copy data from string to buffer
    void* result = memcpy(buffer->bytes, input_string, input_length);
    // return true if memcpy result points to something
    if(result != NULL) {
        return true;
    } else {
        // free memory and return false
        free(buffer->bytes);
        return false;
    }
}

/*
 * given a buffer struct and an open file handle, writes the buffer contents
 * to the file.
 * returns true on success and false on failure.
 */
static bool buffer_to_file(sxbp_buffer_t* buffer, FILE* file_handle) {
    size_t bytes_written = fwrite(
        buffer->bytes, 1, buffer->size, file_handle
    );
    // check amount written was the size of buffer
    if(bytes_written != buffer->size) {
        return false;
    } else {
        return true;
    }
}

/*
 * private function, given a status_t error, returns the string name of the
 * error code
 */
static const char* error_code_string(sxbp_status_t error) {
    switch(error) {
        case SXBP_OPERATION_FAIL:
            return "OPERATION_FAIL";
        case SXBP_MALLOC_REFUSED:
            return "MALLOC_REFUSED";
        case SXBP_IMPOSSIBLE_CONDITION:
            return "IMPOSSIBLE_CONDITION";
        case SXBP_OPERATION_OK:
            return "OPERATION_OK (NO ERROR)";
        case SXBP_STATE_UNKNOWN:
        default:
            return "UNKNOWN ERROR";
    }
}

/*
 * private function, given a deserialise_diagnostic_t error, returns the string
 * name of the error code
 */
static const char* file_error_code_string(sxbp_deserialise_diagnostic_t error) {
    switch(error) {
        case SXBP_DESERIALISE_OK:
            return "DESERIALISE_OK (NO ERROR)";
        case SXBP_DESERIALISE_BAD_HEADER_SIZE:
            return "DESERIALISE_BAD_HEADER_SIZE";
        case SXBP_DESERIALISE_BAD_MAGIC_NUMBER:
            return "DESERIALISE_BAD_MAGIC_NUMBER";
        case SXBP_DESERIALISE_BAD_VERSION:
            return "DESERIALISE_BAD_VERSION";
        case SXBP_DESERIALISE_BAD_DATA_SIZE:
            return "DESERIALISE_BAD_DATA_SIZE";
        default:
            return "UNKNOWN ERROR";
    }
}

/*
 * private function to handle all generic errors, by printing to stderr
 * returns true if there was an error, false if not
 */
static bool handle_error(sxbp_status_t result) {
    // if we had problems, print to stderr and return true
    if(result != SXBP_OPERATION_OK) {
        fprintf(stderr, "Error Code: %s\n", error_code_string(result));
        return true;
    } else {
        // otherwise, return false to say 'no error'
        return false;
    }
}

// enum for representing different spiral render modes
enum spiral_render_mode_t {
    RENDER_MODE_SXP, RENDER_MODE_PBM, RENDER_MODE_PNG,
};

/*
 * private structure, used for supplying many a datum to the callback function
 * passed to plot_spiral()
 */
struct user_data_t {
    // whether to render to sxp, pbm or png format
    enum spiral_render_mode_t render_mode;
    const char* file_path; // path of file to save to
    uint64_t save_line_interval; // save file every this number of lines
};

/*
 * disable GCC warning about the unused parameter, as this is a callback it must
 * include all arguments specified by the library prototype, even if not used
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
/*
 * private function - callback handler for plot_spiral()
 * saves current spiral state to .sxp or .pbm depending on whether in render
 * mode or not, and on how often it is to save output
 */
static void plot_spiral_callback(
    sxbp_spiral_t* spiral, uint64_t latest_line, uint64_t target_line,
    void* user_data_void_pointer
) {
    // cast void pointer to our user data type
    struct user_data_t user_data = *(struct user_data_t*)user_data_void_pointer;
    // first, check if we need to save this time
    if(((spiral->solved_count - 1) % user_data.save_line_interval) == 0) {
        // we do need to save, so check we can open the file first
        FILE* output_file = fopen(user_data.file_path, "wb");
        if(output_file != NULL) {
            // create variable to store return status of serialise operations
            sxbp_status_t result = SXBP_STATE_UNKNOWN;
            // create output buffer
            sxbp_buffer_t output_buffer = {0, 0};
            // Now, check whether it'll be to sxp
            if(user_data.render_mode == RENDER_MODE_SXP) {
                // save to sxp, store result code
                result = sxbp_dump_spiral(
                    *spiral, &output_buffer
                ).status;
            } else if(user_data.render_mode == RENDER_MODE_PBM) {
                /*
                 * render spiral to image, using PBM render function and store
                 * data in buffer - capture return status to check later
                 */
                result = sxbp_render_spiral_image(
                    *spiral, &output_buffer, sxbp_render_backend_pbm
                );
            } else if(user_data.render_mode == RENDER_MODE_PNG) {
                /*
                 * render spiral to image, using PNG render function and store
                 * data in buffer - capture return status to check later
                 */
                result = sxbp_render_spiral_image(
                    *spiral, &output_buffer, sxbp_render_backend_png
                );
            }
            // if all was ok, save to file
            if(result == SXBP_OPERATION_OK) {
                buffer_to_file(&output_buffer, output_file);
                fclose(output_file);
            }
        } else {
            fprintf(
                stderr,
                "Couldn't open file for writing: %s\n",
                user_data.file_path
            );
        }
    }
}
// re-enable all warnings
#pragma GCC diagnostic pop

/*
 * function responsible for actually doing the main work, called by main with
 * options configured via command-line.
 * returns true on success, false on failure.
 */
static bool run(
    bool prepare, bool generate, bool render, bool perfect,
    int perfect_threshold, int line_limit, int total_lines, int save_every,
    const char* image_format, const char* input_string,
    const char* input_file_path, const char* output_file_path
) {
    // make input buffer
    sxbp_buffer_t input_buffer = {0, 0};
    // make output buffer
    sxbp_buffer_t output_buffer = {0, 0};
    // used later for telling if read from input file or string was success
    bool read_ok = false;
    // used later for telling if write of output file was success
    bool write_ok = false;
    // work out whether we're reading from string or file or if neither were given
    if((strcmp(input_file_path, "") == 0) && (strcmp(input_string, "") == 0)) {
        fprintf(stderr, "Neither an input file or an input string were given\n");
        return false;
    } else if(strcmp(input_file_path, "") == 0) {
        // the filepath wasn't given so read from string
        read_ok = string_to_buffer(input_string, &input_buffer);
    } else {
        // the string wasn't given so open the file
        // get input file handle
        FILE* input_file = fopen(input_file_path, "rb");
        if(input_file == NULL) {
            fprintf(stderr, "%s\n", "Couldn't open input file");
            return false;
        }
        // read input file into buffer
        read_ok = file_to_buffer(input_file, &input_buffer);
        // close input file
        fclose(input_file);
        // if read was unsuccessful, don't continue
    }
    if(read_ok == false) {
        fprintf(stderr, "%s\n", "Couldn't read input file/data");
        return false;
    }
    // create initial blank spiral struct
    sxbp_spiral_t spiral = sxbp_blank_spiral();
    // resolve perfection threshold - set to -1 if disabled completely
    int perfection = (perfect == false) ? -1 : perfect_threshold;
    // check error condition (where no actions were specified)
    if((prepare || generate || render) == false) {
        // none of the above. this is an error condition - nothing to be done
        fprintf(stderr, "%s\n", "Nothing to be done!");
        return false;
    }
    // PBM format is default for rendering to image
    enum spiral_render_mode_t default_render_mode = RENDER_MODE_PBM;
    // override render image format if format option given
    if(
        (render == true) &&
        (image_format != NULL) &&
        (strcmp(image_format, "") != 0)
    ) {
        if(strcmp(image_format, "png") == 0) {
            default_render_mode = RENDER_MODE_PNG;
        } else if(strcmp(image_format, "pbm") == 0) {
            // No-op as it's already set to PBM format
            NULL;
        } else {
            // Error, unrecognised file format
            fprintf(
                stderr, "Unsupported image file format: '%s'\n",
                image_format
            );
            return false;
        }
    }
    // otherwise, good to go
    if(prepare) {
        // we must build spiral from raw file first
        if(handle_error(sxbp_init_spiral(input_buffer, &spiral))) {
            // handle errors
            return false;
        }
    } else {
        // otherwise, we must load spiral from file
        sxbp_serialise_result_t result = sxbp_load_spiral(input_buffer, &spiral);
        // if we had problems, print to stderr and quit
        if(result.status != SXBP_OPERATION_OK) {
            fprintf(
                stderr,
                "Error Code:\t\t%s\nFile Error Code:\t%s\n",
                error_code_string(result.status),
                file_error_code_string(result.diagnostic)
            );
            return false;
        }
    }
    if(generate) {
        /*
         * find out how many lines we are to plot
         * this is based on two options: the line_limit and the total_lines
         * arguments.
         */
        // first, check the line_limit argument
        uint64_t lines_to_plot;
        // set to solved count + line limit if set, else spiral size
        lines_to_plot = (
            (line_limit != -1) ? (spiral.solved_count + line_limit) : spiral.size
        );
        // set to total_lines if set and less than current amount
        lines_to_plot = (
            (total_lines != -1 && (uint64_t)total_lines < lines_to_plot) ?
            (uint64_t)total_lines : lines_to_plot
        );
        // we must plot the unsolved lines from spiral file
        sxbp_status_t errors;
        if(save_every > 0) {
            // if we've been asked to save every x lines, we need to use callback
            // build user data for callback
            struct user_data_t user_data = {
                // use default image format if rendering to image
                .render_mode = (
                    (render == false) ? RENDER_MODE_SXP : default_render_mode
                ),
                .file_path = output_file_path,
                .save_line_interval = save_every,
            };
            errors = sxbp_plot_spiral(
                &spiral, perfection, lines_to_plot,
                plot_spiral_callback, (void*)&user_data
            );
        } else {
            // otherwise, no need to use callback
            errors = sxbp_plot_spiral(
                &spiral, perfection, lines_to_plot, NULL, NULL
            );
        }
        // handle errors
        if(handle_error(errors)) {
            // handle errors
            return false;
        }
    }
    if(render) {
        /*
         * render spiral to image, using PBM render function and store
         * data in buffer - handle error if any
         */
        sxbp_status_t error = SXBP_STATE_UNKNOWN;
        // render to whichever image format was specified
        if(default_render_mode == RENDER_MODE_PBM) {
            // render to PBM format
            error = sxbp_render_spiral_image(
                spiral, &output_buffer, sxbp_render_backend_pbm
            );
        } else if(default_render_mode == RENDER_MODE_PNG) {
            // render to PNG format
            error = sxbp_render_spiral_image(
                spiral, &output_buffer, sxbp_render_backend_png
            );
        }
        // handle errors
        if(handle_error(error)) {
            return false;
        }
    } else {
        // otherwise, we must simply dump the spiral as-is
        sxbp_serialise_result_t result = sxbp_dump_spiral(spiral, &output_buffer);
        // if we had problems, print to stderr and quit
        if(result.status != SXBP_OPERATION_OK) {
            fprintf(
                stderr,
                "Error Code:\t\t%s\nFile Error Code:\t%s\n",
                error_code_string(result.status),
                file_error_code_string(result.diagnostic)
            );
            return false;
        }
    }
    // get output file handle
    FILE* output_file = fopen(output_file_path, "wb");
    if(output_file == NULL) {
        fprintf(stderr, "%s\n", "Couldn't open output file");
        return false;
    }
    // now, write output buffer to file
    write_ok = buffer_to_file(&output_buffer, output_file);
    // close output file
    fclose(output_file);
    // free buffers
    free(input_buffer.bytes);
    free(output_buffer.bytes);
    // return success depends on last write
    return write_ok;
}

// main - mostly just process arguments, the bulk of the work is done by run()
int main(int argc, char* argv[]) {
    // status code initially set to -1
    int status_code = -1;
    // build argtable struct for parsing command-line arguments
    // show help
    struct arg_lit* help = arg_lit0("h","help", "show this help and exit");
    // show version
    struct arg_lit* version = arg_lit0(
        "v", "version", "show version of program and library, then exit"
    );
    // flag for if we want to prepare a spiral
    struct arg_lit* prepare = arg_lit0(
        "p", "prepare",
        "prepare a spiral from raw binary data"
    );
    // flag for if we want to generate the solution for a spiral's line lengths
    struct arg_lit* generate = arg_lit0(
        "g", "generate",
        "generate the lengths of a spiral's lines"
    );
    // flag for if we want to render a spiral to imagee
    struct arg_lit* render = arg_lit0(
        "r", "render", "render a spiral to an image"
    );
    struct arg_lit* perfect = arg_lit0(
        "D", "disable-perfection", "allow unlimited optimisations"
    );
    struct arg_int* perfect_threshold = arg_int0(
        "d", "perfection-threshold", NULL, "set optimisation threshold"
    );
    struct arg_int* total_lines = arg_int0(
        "t", "total-lines", NULL, "total number of lines to plot to"
    );
    struct arg_int* line_limit = arg_int0(
        "l", "line-limit", NULL, "plot this many more lines than currently solved"
    );
    struct arg_int* save_every = arg_int0(
        "s", "save-every", NULL, "save to file every this number of lines solved"
    );
    struct arg_str* image_format = arg_str0(
        "f", "image-format", "FORMAT", "which image format to render to (pbm/png)"
    );
    struct arg_str* input_string = arg_str0(
        "S", "string", "STRING",
        "use the given STRING as input data for the spiral"
    );
    // input file path option
    struct arg_file* input = arg_file0(
        "i", "input", NULL, "input file path"
    );
    // output file path option
    struct arg_file* output = arg_file0(
        "o", "output", NULL, "output file path"
    );
    // argtable boilerplate
    struct arg_end* end = arg_end(20);
    void* argtable[] = {
        help, version,
        prepare, generate, render,
        perfect, perfect_threshold, line_limit, total_lines, save_every,
        image_format, input_string,
        input, output, end,
    };
    const char* program_name = "sxbp";
    // check argtable members were allocated successfully
    if(arg_nullcheck(argtable) != 0) {
        // NULL entries were detected, so some allocations failed
        fprintf(
            stderr, "%s\n", "FATAL: Could not allocate all entries for argtable"
        );
        status_code = 2;
    }
    // set default value of perfect_threshold argument
    perfect_threshold->ival[0] = 1;
    // set default value of line_limit, total_lines and save_every arguments
    line_limit->ival[0] = -1;
    total_lines->ival[0] = -1;
    save_every->ival[0] = -1;
    // parse arguments
    int count_errors = arg_parse(argc, argv, argtable);
    // if we asked for the version, show it
    if(version->count > 0) {
        printf(
            "%s %s (using libsaxbospiral %s)\n",
            program_name, SXBP_VERSION_STRING, LIB_SXBP_VERSION.string
        );
        status_code = 0;
    }
    // if parser returned any errors, display them and set return code to 1
    if(count_errors > 0) {
        arg_print_errors(stderr, end, program_name);
        status_code = 1;
    }
    // set return code to 0 if we asked for help
    if(help->count > 0) {
        status_code = 0;
    }
    // display usage information if we asked for help or got arguments wrong
    if((count_errors > 0) || (help->count > 0)) {
        printf("Usage: %s", program_name);
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "  %-32s %s\n");
    }
    // if at this point status_code is not -1, clean up then return early
    if(status_code != -1) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return status_code;
    }
    // otherwise, carry on...
    // now, call run with options from command-line
    bool result = run(
        (prepare->count > 0) ? true : false,
        (generate->count > 0) ? true : false,
        (render->count > 0) ? true : false,
        (perfect->count > 0) ? false : true,
        perfect_threshold->ival[0],
        line_limit->ival[0],
        total_lines->ival[0],
        save_every->ival[0],
        image_format->sval[0],
        input_string->sval[0],
        *input->filename,
        *output->filename
    );
    // free argtable struct
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    // return appropriate status code based on success/failure
    return (result) ? 0 : 1;
}

#ifdef __cplusplus
} // extern "C"
#endif
