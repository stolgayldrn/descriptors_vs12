#include "ES_image.h"

using namespace std;


int getJSON_new_image(Image_Info* my_II, Path* p, json_t* my_source, string words_str)
{
	try{
		json_object_set_new(my_source, "data_set", json_string(my_II->dataSet.c_str()));
		json_object_set_new(my_source, "data_sub_set", json_string(my_II->dataSubSet.c_str()));
		json_object_set_new(my_source, "url", json_string(my_II->url.c_str()));
		json_object_set_new(my_source, "file_name", json_string(my_II->fileName.c_str()));
		json_object_set_new(my_source, "descriptor_type", json_string(my_II->descriptorType.c_str()));
		json_object_set_new(my_source, "source_type", json_string(my_II->source_type.c_str()));
		json_object_set_new(my_source, "encoding", json_string(my_II->encoding.c_str()));
		json_object_set_new(my_source, "height", json_string(int2string(my_II->height).c_str()));
		json_object_set_new(my_source, "width", json_string(int2string(my_II->width).c_str()));
		json_object_set_new(my_source, "disk_path", json_string(my_II->path.c_str()));
		json_object_set_new(my_source, "words_string", json_string(words_str.c_str()));
	}
	catch (exception e){
		cout << "# ERR: Elasticsearch Exception in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
	}
	if (!json_is_object(my_source)){
		fprintf(stderr, "error: commits is not an array\n");
		return 0;
	}
	return 1;
}

int ES_commit(ES_params* my_ES, json_t* my_source, const char * ES_id, string fileName)
{
	CURL *curl = curl_easy_init();
	//char *userPWD = "writer:writeme";

	string ES_new_object_url = my_ES->url + "/" + my_ES->index + "/" + my_ES->type;
	struct curl_slist *headers = NULL;
	size_t json_flags = 0;
	/* set content type */
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	// Response information.
	int httpCode(0);
	std::unique_ptr<std::string> httpData(new std::string());

	if (curl) {
		try{
			CURLcode res;
			/* set curl options */
			curl_easy_setopt(curl, CURLOPT_URL, ES_new_object_url.c_str());
			//curl_easy_setopt(curl, CURLOPT_USERPWD, userPWD);
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_dumps(my_source, json_flags));
			// Hook up data handling function.
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
			// Hook up data container (will be passed as the last parameter to the
			// callback handling function).  Can be any pointer type, since it will
			// internally be passed as a void pointer.
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

			res = curl_easy_perform(curl);
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
			/* cleanup curl handle */
			curl_easy_cleanup(curl);
			/* free headers */
			curl_slist_free_all(headers);
		}
		catch (exception e){
			printf("\nElasticsearch error: ", e.what());
			return 0;
		}
	}

	if (httpCode == 201){
		json_t *rootRes;
		json_error_t error;
		try
		{
			rootRes = json_loads(httpData->data(), 0, &error);
			if (!rootRes){
				fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
				return 1;
			}
			int boolean;
			const char *str;
			json_unpack(rootRes, "{s:s}", "_id", &str);
			ES_id = str;
			//printf("\nES::::New image is committed to image_search_test\\akaze\\%s.", fileName.c_str());
			httpData->clear();
			httpData->shrink_to_fit();
			json_decref(rootRes);
			return 1;
		}
		catch (exception e){
			printf("\nElasticsearch error: ", e.what());
			return 0;
		}
	}
	else{
		printf("\nElasticsearch error: httpCode, %d", httpCode);
		return 0;
	}
}

int getJSON_query_image(json_t* my_source, string words_str)
{
	json_t *query = json_object();
	json_t *match = json_object();
	try	{
		json_object_set_new(match, "words_string", json_string(words_str.c_str()));
		json_object_set_new(query, "match", match);
		json_object_set_new(my_source, "query", query);
		return 1;
	}
	catch (exception e){
		cout << "# ERR: Elasticsearch Exception in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << ", ES State: " << e.what() << " )" << endl;
		return 0;
	}
}

int ES_post_query(ES_params* my_ES, vector<vector<string>>& ES_results, json_t* my_source, Image_Info* my_II)
{
	vector<string> ES_result;
	ES_result.push_back(my_II->fileName.c_str());
	CURL *curl = curl_easy_init();
	//char *userPWD = "writer:writeme";
	string ES_new_object_url = my_ES->url + "/" + my_ES->index + "/" + "_search";
	struct curl_slist *headers = NULL;
	size_t json_flags = 0;
	/* set content type */
	headers = curl_slist_append(headers, "Accept: application/json");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	// Response information.
	int httpCode(0);
	std::unique_ptr<std::string> httpData(new std::string());
	if (curl){
		try{
			CURLcode res;
			/* set curl options */
			curl_easy_setopt(curl, CURLOPT_URL, ES_new_object_url.c_str());
			//curl_easy_setopt(curl, CURLOPT_USERPWD, userPWD);
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_dumps(my_source, json_flags));
			// Hook up data handling function.
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
			// Hook up data container (will be passed as the last parameter to the callback handling function).  
			// Can be any pointer type, since it will internally be passed as a void pointer.
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

			res = curl_easy_perform(curl);
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
			/* cleanup curl handle */
			curl_easy_cleanup(curl);
			/* free headers */
			curl_slist_free_all(headers);
		}
		catch (exception e){
			printf("\nElasticsearch error: ", e.what());
			return 0;
		}
	}
	if (httpCode == 200){
		json_t *rootRes, *hits, *hits2;
		json_error_t error;
		rootRes = json_loads(httpData->data(), 0, &error);
		if (!rootRes){
			fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
			return 0;
		}
		hits = json_object_get(rootRes, "hits");
		hits2 = json_object_get(hits, "hits");
		if (!json_is_array(hits2)){
			fprintf(stderr, "error: hits2 is not an array\n");
			return 0;
		}
		else{
			for (int i = 0; i < 10; i++){
				if (i < json_array_size(hits2)){
					json_t *data, *sourceRes, *image_ID;
					data = json_array_get(hits2, i);
					const char *message_text;
					if (!json_is_object(data)){
						fprintf(stderr, "error: commit data %d is not an object\n", i + 1);
						return 1;
					}
					sourceRes = json_object_get(data, "_source");
					image_ID = json_object_get(sourceRes, "file_name");
					message_text = json_string_value(image_ID);
					ES_result.push_back(message_text);
					//releases
					json_object_clear(data);
					json_object_clear(sourceRes);
					json_object_clear(image_ID);
				}
				else{
					ES_result.push_back("-1");
				}
			}
			//releases			
			json_object_clear(rootRes);
			json_object_clear(hits);
			json_object_clear(hits2);
			ES_results.push_back(ES_result);
			ES_result.clear();
			return 1;
		}
	}
	else{
		return 0;
	}
}

int initialization(TVoctreeVLFeat* VT, Path* p)
{
	try{
		VT->init_read(p->VocTree.c_str());
		return 1;
	}
	catch (exception e){
		printf("\nInitialization error: ", e.what());
		return 0;
	}
}
