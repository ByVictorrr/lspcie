
/* Acts like the number of T's */

void 
show_dev_json_obj(struct device *d)
{
    static json_value *arr=NULL;
    struct version_item *vitemss[3]={NULL}, *vitems=NULL; /* 0= drv, 1=fwv, 2=optv*/
    #define MAX_BUFF 1024
    char buff[MAX_BUFF];
    json_value *dev_obj = json_object_new(0);

    memset(buff, 0, MAX_BUFF);
    // Step 1 - pci address output
    set_slot_name(d, buff);
    json_object_push(dev_obj, "pci address", json_string_new(buff));
    // Step  2 - phys slot 
    memset(buff, 0, MAX_BUFF);
    set_phy_slot(d, buff);
    json_object_push(dev_obj, "slot #", json_string_new(buff));
    // Step 3 - card info 
    memset(buff, 0, MAX_BUFF);
    set_phy_slot(d, buff);
    json_object_push(dev_obj, "card info", json_string_new(buff));
    // Step 4 - vendor name
    memset(buff, 0, MAX_BUFF);
    set_vendor(d, buff);
    json_object_push(dev_obj, "vendor", json_string_new(buff));
    // Step 5 - driver
    memset(buff, 0, MAX_BUFF);
    set_driver(d, buff);
    json_object_push(dev_obj, "driver", json_string_new(buff));
    return dev_obj;
}