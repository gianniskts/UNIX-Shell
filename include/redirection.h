void checkRedirection(char** tokens, bool* redirect_input, bool* redirect_output, char** input_file, char** output_file, bool* append_output);

void handleRedirection(bool redirect_input, bool redirect_output, char* input_file, char* output_file, bool append_output);
