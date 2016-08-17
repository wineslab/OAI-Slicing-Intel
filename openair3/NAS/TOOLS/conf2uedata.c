#include <stdio.h>  // perror, printf, fprintf, snprintf
#include <stdlib.h> // exit, free
#include <string.h> // memset, strncpy
#include <getopt.h>

#include "conf2uedata.h"
#include "memory.h"
#include "utils.h"
#include "display.h"
#include "fs.h"
#include "conf_emm.h"
#include "conf_user_data.h"
#include "conf_usim.h"

int *ucplmn = NULL;
int *oplmn = NULL;
int *ocplmn = NULL;
int *fplmn = NULL;
int *ehplmn = NULL;

int hplmn_index = 0;
int plmn_nb = 0;
int ucplmn_nb = 0;
int oplmn_nb = 0;
int ocplmn_nb = 0;
int fplmn_nb = 0;
int ehplmn_nb = 0;

plmn_conf_param_t* user_plmn_list=NULL;
network_record_t* user_network_record_list = NULL;

int main(int argc, char**argv) {
	int option;
    const char* conf_file = NULL;
    const char* output_dir = NULL;
    const char options[]="c:o:h";

    while ((option = getopt(argc, argv, options)) != -1) {
		switch (option) {
		case 'c':
			conf_file = optarg;
			break;
		case 'o':
			output_dir = optarg;
			break;
		case 'h':
			_display_usage();
			return EXIT_SUCCESS;
			break;
		default:
			break;
		}
	}

	if (output_dir == NULL ) {
		printf("No output option found\n");
		_display_usage();
		return EXIT_FAILURE;
	}

    if ( conf_file == NULL ) {
		printf("No Configuration file is given\n");
		_display_usage();
		return EXIT_FAILURE;
	}

    if ( parse_config_file(output_dir, conf_file) == EXIT_FAILURE ) {
        exit(EXIT_FAILURE);
    }

    display_data_from_directory(output_dir);

	exit(EXIT_SUCCESS);
}

int parse_config_file(const char *output_dir, const char *conf_filename) {
	int rc = EXIT_SUCCESS;
    int ret;
    int ue_nb = 0;
    config_setting_t *root_setting = NULL;
    config_setting_t *ue_setting = NULL;
    config_setting_t *all_plmn_setting = NULL;
    char user[10];
    config_t cfg;

    ret = get_config_from_file(conf_filename, &cfg);
    if (ret == EXIT_FAILURE) {
        exit(1);
    }

    root_setting = config_root_setting(&cfg);
    ue_nb = config_setting_length(root_setting) - 1;

    all_plmn_setting = config_setting_get_member(root_setting, PLMN);
    if (all_plmn_setting == NULL) {
        printf("NO PLMN SECTION...EXITING...\n");
        return (EXIT_FAILURE);
    }

    rc = parse_plmns(all_plmn_setting);
    if (rc == EXIT_FAILURE) {
        return rc;
    }
    fill_network_record_list();

    for (int i = 0; i < ue_nb; i++) {
	    emm_nvdata_t emm_data;

	    user_nvdata_t user_data;
	    user_data_conf_t user_data_conf;

	    usim_data_t usim_data;
	    usim_data_conf_t usim_data_conf;

        sprintf(user, "%s%d", UE, i);

        ue_setting = config_setting_get_member(root_setting, user);
        if (ue_setting == NULL) {
            printf("Check UE%d settings\n", i);
            return EXIT_FAILURE;
        }

        rc = parse_ue_plmn_param(ue_setting, i, &usim_data_conf.hplmn);
        if (rc != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        rc = parse_ue_user_data(ue_setting, i, &user_data_conf);
        if (rc != EXIT_SUCCESS) {
            printf("Problem in USER section for UE%d. EXITING...\n", i);
            return EXIT_FAILURE;
        }
        gen_user_data(&user_data_conf, &user_data);
        write_user_data(output_dir, i, &user_data);

        rc = parse_ue_sim_param(ue_setting, i, &usim_data_conf);
        if (rc != EXIT_SUCCESS) {
            printf("Problem in SIM section for UE%d. EXITING...\n", i);
            return EXIT_FAILURE;
        }
        gen_usim_data(&usim_data_conf, &usim_data);
        write_usim_data(output_dir, i, &usim_data);

        gen_emm_data(&emm_data, usim_data_conf.hplmn, usim_data_conf.msin);
        write_emm_data(output_dir, i, &emm_data);

     }
    config_destroy(&cfg);
	return(EXIT_SUCCESS);
}

int get_config_from_file(const char *filename, config_t *config) {
    config_init(config);
    if (filename == NULL) {
        // XXX write error message ?
        exit(EXIT_FAILURE);
    }

    /* Read the file. If there is an error, report it and exit. */
    if (!config_read_file(config, filename)) {
        fprintf(stderr, "Cant read config file '%s': %s\n", filename,
                config_error_text(config));
        if ( config_error_type(config) == CONFIG_ERR_PARSE ) {
            fprintf(stderr, "This is line %d\n", config_error_line(config));
        }
        config_destroy(config);
        return (EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}


int parse_plmn_param(config_setting_t *plmn_setting, int index) {
	int rc = 0;
	rc = config_setting_lookup_string(plmn_setting,
	FULLNAME, &user_plmn_list[index].fullname);
	if (rc != 1) {
		printf("Check PLMN%d FULLNAME. Exiting\n", index);
		return EXIT_FAILURE;
	}
	rc = config_setting_lookup_string(plmn_setting,
	SHORTNAME, &user_plmn_list[index].shortname);
	if (rc != 1) {
		printf("Check PLMN%d SHORTNAME. Exiting\n", index);
		return EXIT_FAILURE;
	}
	rc = config_setting_lookup_string(plmn_setting,
	MNC, &user_plmn_list[index].mnc);
	if (rc != 1 || strlen(user_plmn_list[index].mnc) > 3
			|| strlen(user_plmn_list[index].mnc) < 2) {
		printf("Check PLMN%d MNC. Exiting\n", index);
		return EXIT_FAILURE;
	}
	rc = config_setting_lookup_string(plmn_setting,
	MCC, &user_plmn_list[index].mcc);
	if (rc != 1 || strlen(user_plmn_list[index].mcc) != 3) {
		printf("Check PLMN%d MCC. Exiting\n", index);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int parse_plmns(config_setting_t *all_plmn_setting) {
	config_setting_t *plmn_setting = NULL;
	char plmn[10];
	int rc = EXIT_SUCCESS;
	plmn_nb = config_setting_length(all_plmn_setting);
	user_plmn_list = malloc(sizeof(plmn_conf_param_t) * plmn_nb);
	user_network_record_list = malloc(sizeof(network_record_t) * plmn_nb);
	for (int i = 0; i < plmn_nb; i++) {
		memset(&user_network_record_list[i], 0xff, sizeof(network_record_t));
		memset(&user_plmn_list[i], 0xff, sizeof(plmn_conf_param_t));
	}
	for (int i = 0; i < plmn_nb; i++) {
		sprintf(plmn, "%s%d", PLMN, i);
		plmn_setting = config_setting_get_member(all_plmn_setting, plmn);
		if (plmn_setting != NULL) {
			rc = parse_plmn_param(plmn_setting, i);
			if (rc == EXIT_FAILURE) {
				return rc;
			}
		} else {
			printf("Problem in PLMN%d. Exiting...\n", i);
			return EXIT_FAILURE;
		}
	}
	return rc;
}

int parse_ue_plmn_param(config_setting_t *ue_setting, int user_id, const char **h) {
	int rc = EXIT_SUCCESS;
	const char *hplmn;
	config_setting_t *setting = NULL;
	rc = config_setting_lookup_string(ue_setting, HPLMN, h);
	if (rc != 1) {
		printf("Check HPLMN section for UE%d. Exiting\n", user_id);
		return EXIT_FAILURE;
	}
	hplmn = *h;
	if (get_plmn_index(hplmn) == -1) {
		printf("HPLMN for UE%d is not defined in PLMN section. Exiting\n",
				user_id);
		return EXIT_FAILURE;
	}
	setting = config_setting_get_member(ue_setting, UCPLMN);
	if (setting == NULL) {
		printf("Check UCPLMN section for UE%d. Exiting\n", user_id);
		return EXIT_FAILURE;
	}
	rc = fill_ucplmn(setting, user_id);
	if (rc != EXIT_SUCCESS) {
		printf("Check UCPLMN section for UE%d. Exiting\n", user_id);
		return EXIT_FAILURE;
	}
	setting = config_setting_get_member(ue_setting, OPLMN);
	if (setting == NULL) {
		printf("Check OPLMN section for UE%d. Exiting\n", user_id);
		return EXIT_FAILURE;
	}
	rc = fill_oplmn(setting, user_id);
	if (rc != EXIT_SUCCESS) {
		printf("Check OPLMN section for UE%d. Exiting\n", user_id);
		return EXIT_FAILURE;
	}
	setting = config_setting_get_member(ue_setting, OCPLMN);
	if (setting == NULL) {
		printf("Check OCPLMN section for UE%d. Exiting\n", user_id);
		return EXIT_FAILURE;
	}
	rc = fill_ocplmn(setting, user_id);
	if (rc != EXIT_SUCCESS) {
		printf("Check OCPLMN section for UE%d. Exiting\n", user_id);
		return EXIT_FAILURE;
	}
	setting = config_setting_get_member(ue_setting, FPLMN);
	if (setting == NULL) {
		printf("Check FPLMN section for UE%d. Exiting\n", user_id);
		return EXIT_FAILURE;
	}
	rc = fill_fplmn(setting, user_id);
	if (rc != EXIT_SUCCESS) {
		printf("Check FPLMN section for UE%d. Exiting\n", user_id);
		return EXIT_FAILURE;
	}
	setting = config_setting_get_member(ue_setting, EHPLMN);
	if (setting == NULL) {
		printf("Check EHPLMN section for UE%d. Exiting\n", user_id);
		return EXIT_FAILURE;
	}
	rc = fill_ehplmn(setting, user_id);
	if (rc != EXIT_SUCCESS) {
		printf("Check EHPLMN section for UE%d. Exiting\n", user_id);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int fill_ucplmn(config_setting_t* setting, int user_id) {
	int rc;
	ucplmn_nb = config_setting_length(setting);
	ucplmn = malloc(ucplmn_nb * sizeof(int));
	for (int i = 0; i < ucplmn_nb; i++) {
		const char *mccmnc = config_setting_get_string_elem(setting, i);
		if (mccmnc == NULL) {
			printf("Check UCPLMN section for UE%d. Exiting\n", user_id);
			return EXIT_FAILURE;
		}
		rc = get_plmn_index(mccmnc);
		if (rc == -1) {
			printf("The PLMN %s is not defined in PLMN section. Exiting...\n",
					mccmnc);
			return EXIT_FAILURE;
		}
		ucplmn[i] = rc;
	}
	return EXIT_SUCCESS;
}
int fill_oplmn(config_setting_t* setting, int user_id) {
	int rc;
	oplmn_nb = config_setting_length(setting);
	oplmn = malloc(oplmn_nb * sizeof(int));
	for (int i = 0; i < oplmn_nb; i++) {
		const char *mccmnc = config_setting_get_string_elem(setting, i);
		if (mccmnc == NULL) {
			printf("Check OPLMN section for UE%d. Exiting\n", user_id);
			return EXIT_FAILURE;
		}
		rc = get_plmn_index(mccmnc);
		if (rc == -1) {
			printf("The PLMN %s is not defined in PLMN section. Exiting...\n",
					mccmnc);
			return EXIT_FAILURE;
		}
		oplmn[i] = rc;
	}
	return EXIT_SUCCESS;
}
int fill_ocplmn(config_setting_t* setting, int user_id) {
	int rc;
	ocplmn_nb = config_setting_length(setting);
	ocplmn = malloc(ocplmn_nb * sizeof(int));
	for (int i = 0; i < ocplmn_nb; i++) {
		const char *mccmnc = config_setting_get_string_elem(setting, i);
		if (mccmnc == NULL) {
			printf("Check OCPLMN section for UE%d. Exiting\n", user_id);
			return EXIT_FAILURE;
		}
		rc = get_plmn_index(mccmnc);
		if (rc == -1) {
			printf("The PLMN %s is not defined in PLMN section. Exiting...\n",
					mccmnc);
			return EXIT_FAILURE;
		}
		ocplmn[i] = rc;
	}
	return EXIT_SUCCESS;
}
int fill_fplmn(config_setting_t* setting, int user_id) {
	int rc;
	fplmn_nb = config_setting_length(setting);
	fplmn = malloc(fplmn_nb * sizeof(int));
	for (int i = 0; i < fplmn_nb; i++) {
		const char *mccmnc = config_setting_get_string_elem(setting, i);
		if (mccmnc == NULL) {
			printf("Check FPLMN section for UE%d. Exiting\n", user_id);
			return EXIT_FAILURE;
		}
		rc = get_plmn_index(mccmnc);
		if (rc == -1) {
			printf("The PLMN %s is not defined in PLMN section. Exiting...\n",
					mccmnc);
			return EXIT_FAILURE;
		}
		fplmn[i] = rc;
	}
	return EXIT_SUCCESS;
}
int fill_ehplmn(config_setting_t* setting, int user_id) {
	int rc;
	ehplmn_nb = config_setting_length(setting);
	ehplmn = malloc(ehplmn_nb * sizeof(int));
	for (int i = 0; i < ehplmn_nb; i++) {
		const char *mccmnc = config_setting_get_string_elem(setting, i);
		if (mccmnc == NULL) {
			printf("Check EHPLMN section for UE%d. Exiting\n", user_id);
			return EXIT_FAILURE;
		}
		rc = get_plmn_index(mccmnc);
		if (rc == -1) {
			printf("The PLMN %s is not defined in PLMN section. Exiting...\n",
					mccmnc);
			return EXIT_FAILURE;
		}
		ehplmn[i] = rc;
	}
	return EXIT_SUCCESS;
}

int get_plmn_index(const char * mccmnc) {
	char mcc[4];
	char mnc[strlen(mccmnc) - 2];
	strncpy(mcc, mccmnc, 3);
	mcc[3] = '\0';
	strncpy(mnc, mccmnc + 3, 3);
	mnc[strlen(mccmnc) - 3] = '\0';
	for (int i = 0; i < plmn_nb; i++) {
		if (strcmp(user_plmn_list[i].mcc, mcc) == 0) {
			if (strcmp(user_plmn_list[i].mnc, mnc) == 0) {
				return i;
			}
		}
	}
	return -1;
}

void fill_network_record_list() {
	for (int i = 0; i < plmn_nb; i++) {
		strcpy(user_network_record_list[i].fullname,
				user_plmn_list[i].fullname);
		strcpy(user_network_record_list[i].shortname,
				user_plmn_list[i].shortname);
		char num[6];
		sprintf(num, "%s%s", user_plmn_list[i].mcc, user_plmn_list[i].mnc);
		user_network_record_list[i].num = atoi(num);
		user_network_record_list[i].plmn.MCCdigit2 = user_plmn_list[i].mcc[1];
		user_network_record_list[i].plmn.MCCdigit1 = user_plmn_list[i].mcc[0];
		user_network_record_list[i].plmn.MCCdigit3 = user_plmn_list[i].mcc[2];
		user_network_record_list[i].plmn.MNCdigit2 = user_plmn_list[i].mnc[1];
		user_network_record_list[i].plmn.MNCdigit1 = user_plmn_list[i].mnc[0];
		user_network_record_list[i].tac_end = 0xfffd;
		user_network_record_list[i].tac_start = 0x0001;
		if (strlen(user_plmn_list[i].mnc) > 2) {
			user_network_record_list[i].plmn.MNCdigit3 =
					user_plmn_list[i].mnc[2];
		}

	}
}

/*
 * Displays command line usage
 */
void _display_usage(void) {
	fprintf(stderr, "usage: conf2uedata [OPTION] [directory] ...\n");
	fprintf(stderr, "\t[-c]\tConfig file to use\n");
	fprintf(stderr, "\t[-o]\toutput file directory\n");
	fprintf(stderr, "\t[-h]\tDisplay this usage\n");
}
