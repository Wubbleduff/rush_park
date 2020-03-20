
#pragma once

void record_key_event(int vk_code, bool state);
void record_mouse_event(int vk_code, bool state);

void serialize_input(void **out_stream, int *out_bytes);

void deserialize_input(void *stream, int bytes);

