#include "stratum_message.h"
#include "esp_log.h"
#include "jobs.h"

#include "job.h"
#include "esp_log.h"

static const char *TAG = "JOB";
static parsed_message message = {};

void (*methodHandlers[NUM_METHODS])() = {
    process_mining_notify, 
    process_mining_set_difficulty,  
    process_mining_set_version_mask
};

// this would be the one function that every other component would call
// depending on if it's processed on this cpu, external esp32, fpga, etc
// it would do different things
// we'll need to set up a config so the make files compile the right file based on the target
// or we can do a big switch !

void process_message(const char *message_json) 
{
    ESP_LOGI(TAG, "Received message: %s", message_json);
    parse_message(message_json);
}


/*
Functions only for processing stratum messages: On same ESP32 as pool connection
*/

void process_mining_notify()
{
    mining_notify * new_work = malloc(sizeof(mining_notify));
    
    cJSON * params = cJSON_GetObjectItem(json, "params");
    new_work->job_id = strdup(cJSON_GetArrayItem(params, 0)->valuestring);
    new_work->prev_block_hash = strdup(cJSON_GetArrayItem(params, 1)->valuestring);
    new_work->coinbase_1 = strdup(cJSON_GetArrayItem(params, 2)->valuestring);
    new_work->coinbase_2 = strdup(cJSON_GetArrayItem(params, 3)->valuestring);

    cJSON * merkle_branch = cJSON_GetArrayItem(params, 4);
    new_work->n_merkle_branches = cJSON_GetArraySize(merkle_branch);
    if (new_work->n_merkle_branches > MAX_MERKLE_BRANCHES) {
        printf("Too many Merkle branches.\n");
        abort();
    }
    new_work->merkle_branches = malloc(HASH_SIZE * new_work->n_merkle_branches);
    for (size_t i = 0; i < new_work->n_merkle_branches; i++) {
        hex2bin(cJSON_GetArrayItem(merkle_branch, i)->valuestring, new_work->merkle_branches + HASH_SIZE * i, HASH_SIZE * 2);
    }

    new_work->version = strtoul(cJSON_GetArrayItem(params, 5)->valuestring, NULL, 16);
    new_work->target = strtoul(cJSON_GetArrayItem(params, 6)->valuestring, NULL, 16);
    new_work->ntime = strtoul(cJSON_GetArrayItem(params, 7)->valuestring, NULL, 16);

    message->mining_notification = new_work;

    // params can be varible length
    int paramsLength = cJSON_GetArraySize(params);
    int value = cJSON_IsTrue(cJSON_GetArrayItem(params, paramsLength - 1));
    message->should_abandon_work = value;
}

void process_mining_set_difficulty()
{
    cJSON * params = cJSON_GetObjectItem(json, "params");
    uint32_t difficulty = cJSON_GetArrayItem(params, 0)->valueint;

    message->new_difficulty = difficulty;
}

void process_mining_set_version_mask()
{
    cJSON * params = cJSON_GetObjectItem(json, "params");
    uint32_t version_mask = strtoul(cJSON_GetArrayItem(params, 0)->valuestring, NULL, 16);
    message->version_mask = version_mask;
}

void parse_request(cJSON * method_json)
{
    if (cJSON_IsString(method_json)) {
        if (strcmp(method_json->valuestring, "mining.notify") == 0) {
            message->method = MINING_NOTIFY;
        } else if (strcmp(method_json->valuestring, "mining.set_difficulty") == 0) {
            message->method = MINING_SET_DIFFICULTY;
        } else if (strcmp(method_json->valuestring, "mining.set_version_mask") == 0) {
            message->method = MINING_SET_VERSION_MASK;
        } else {
            ESP_LOGI(TAG, "unhandled method in stratum message: %s", method_json->valuestring);
            // should this return error?
        }
    }

         
    if (action >= 0 && action < NUM_ACTIONS) {
        actionFunctions[action]();
    } else {
        printf("Invalid action!\n");
    }
 
}

void parse_response(cJSON * json)
{
    cJSON * result_json = cJSON_GetObjectItem(json, "result");
    cJSON * error_json = cJSON_GetObjectItem(json, "error");

    //Result is null - error
    if (result_json == NULL) {
        message->response_success = false;

    //Error sent - error
    } else if (!cJSON_IsNull(error_json)) {
        if (parsed_id < 5) {
            result = STRATUM_RESULT_SETUP;
        } else {
            result = STRATUM_RESULT;
        }
        message->response_success = false;

    //if the result is a boolean, then parse it
    } else if (cJSON_IsBool(result_json)) {
        // if the id is less than 5 it was one of the set up ids
        if (parsed_id < 5) {
            result = STRATUM_RESULT_SETUP;
        } else {
            result = STRATUM_RESULT;
        }
        if (cJSON_IsTrue(result_json)) {
            message->response_success = true;
        } else {
            message->response_success = false;
        }
    
    //if the id is STRATUM_ID_SUBSCRIBE parse it
    } else if (parsed_id == STRATUM_ID_SUBSCRIBE) {
        result = STRATUM_RESULT_SUBSCRIBE;

        cJSON * extranonce2_len_json = cJSON_GetArrayItem(result_json, 2);
        if (extranonce2_len_json == NULL) {
            ESP_LOGE(TAG, "Unable to parse extranonce2_len: %s", result_json->valuestring);
            message->response_success = false;
            goto done;
        }
        message->extranonce_2_len = extranonce2_len_json->valueint;

        cJSON * extranonce_json = cJSON_GetArrayItem(result_json, 1);
        if (extranonce_json == NULL) {
            ESP_LOGE(TAG, "Unable parse extranonce: %s", result_json->valuestring);
            message->response_success = false;
            goto done;
        }
        message->extranonce_str = malloc(strlen(extranonce_json->valuestring) + 1);
        strcpy(message->extranonce_str, extranonce_json->valuestring);
        message->response_success = true;

    //if the id is STRATUM_ID_CONFIGURE parse it
    } else if (parsed_id == STRATUM_ID_CONFIGURE) {
        cJSON * mask = cJSON_GetObjectItem(result_json, "version-rolling.mask");
        if (mask != NULL) {
            result = STRATUM_RESULT_VERSION_MASK;
            message->version_mask = strtoul(mask->valuestring, NULL, 16);
        } else {
            ESP_LOGI(TAG, "error setting version mask: %s", stratum_json);
        }

    } else {
        ESP_LOGI(TAG, "unhandled result in stratum message: %s", stratum_json);
    }

}

void parse_message(const char * message_json)
{
    // is valid json
    cJSON * json = cJSON_Parse(message_json);
    if (json == NULL) {
        ESP_LOGE(TAG, "Error parsing JSON: %s", message_json);
        return;
    }

    /*
    suggested new flow
    1. parse method
    2. if method isn't null create a request object
    3. if method is null create a response object
    4. parse id
    5. if request - parse request
    6. if response - check for error
    7. if no error parse result
    */

    // parse id 
    // TODO what happens if no id?
    cJSON * id_json = cJSON_GetObjectItem(json, "id");
    int64_t parsed_id = -1;
    if (id_json != NULL && cJSON_IsNumber(id_json)) {
        parsed_id = id_json->valueint;
    }
    message->message_id = parsed_id;

    
    // parse method
    cJSON * method_json = cJSON_GetObjectItem(json, "method");

    //if there is a method this is a server request
    if (method_json != NULL && cJSON_IsString(method_json)) {
        parse_request(method_json);
    //if none then it's a server response with a result or error
    } else {
        parse_response(json)
    }

    //message->method = result;

    // whats this?
    done:
    cJSON_Delete(json);
}