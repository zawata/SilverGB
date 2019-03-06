//generated with cfg_class_gen.py
#include <glib-2.0/glib.h>

class Configuration_Fields {
    GKeyFile *cfg_file;
public:
    Configuration_Fields(GKeyFile *file): cfg_file(file) {}

    /**
      * GUI Access Class
      */
    struct __GUI {
        __GUI(Configuration_Fields *parent): parent(parent) {}

        /**
          * nfd_path
          */
        bool get_nfd_path() {
            return (bool)g_key_file_get_boolean(parent->cfg_file, "GUI", "nfd_path", nullptr);
        }

        void set_nfd_path(bool data) {
            g_key_file_set_boolean(parent->cfg_file, "GUI", "nfd_path", data);
        }

    private:
        Configuration_Fields *parent;
    } GUI = __GUI(this);

    /**
      * BIOS Access Class
      */
    struct __BIOS {
        __BIOS(Configuration_Fields *parent): parent(parent) {}

        /**
          * bios_filepath
          */
        const char * get_bios_filepath() {
            return (const char *)g_key_file_get_string(parent->cfg_file, "BIOS", "bios_filepath", nullptr);
        }

        void set_bios_filepath(const char * data) {
            g_key_file_set_string(parent->cfg_file, "BIOS", "bios_filepath", data);
        }

        /**
          * bios_loaded
          */
        bool get_bios_loaded() {
            return (bool)g_key_file_get_boolean(parent->cfg_file, "BIOS", "bios_loaded", nullptr);
        }

        void set_bios_loaded(bool data) {
            g_key_file_set_boolean(parent->cfg_file, "BIOS", "bios_loaded", data);
        }

    private:
        Configuration_Fields *parent;
    } BIOS = __BIOS(this);

};