#ifndef slic3r_ModelBackup_hpp_
#define slic3r_ModelBackup_hpp_

#include <string>
#include <functional>

namespace Slic3r {

class ModelObject;
class Model;

extern void save_object_mesh(ModelObject& object);
extern void delete_object_mesh(ModelObject& object);
extern void backup_soon();
extern void remove_backup(Model& model, bool removeAll);
extern void set_backup_interval(long interval);
extern void set_backup_callback(std::function<void(int)> callback);
extern void run_backup_ui_tasks();
extern bool has_restore_data(std::string & path, std::string & origin);
extern void put_other_changes();
extern void clear_other_changes(bool backup);
extern bool has_other_changes(bool backup);

class SaveObjectGaurd {
public:
    SaveObjectGaurd(ModelObject& object);
    ~SaveObjectGaurd();
};

} // namespace Slic3r

#endif /* slic3r_ModelBackup_hpp_ */
