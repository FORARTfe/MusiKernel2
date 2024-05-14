#!/usr/bin/python3
"""
This file is part of the MusiKernel project, Copyright MusiKernel Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
"""

import os

global_pydaw_version_string = open("../major-version.txt").read().strip()

f_dir_name = os.path.abspath(os.path.dirname(__file__))
f_po_file = "{}/{}.po".format(f_dir_name, global_pydaw_version_string)
f_gtranz_file = "{}/locale/goo.tranz".format(f_dir_name)
f_path = "{}/locale/src".format(f_dir_name)

def check_escape_sequences(a_file):
    f_dict = {}
    with open(a_file) as f_file:
        f_arr = f_file.read().split("msgid")
        for f_kvp in f_arr[1:]:
            f_key, f_val = f_kvp.split("msgstr")
            #TODO:  If a '#' char is actually in the string this breaks
            f_val = f_val.split("#")[0]
            f_dict[f_key.strip()] = f_val.strip()
    for k, v in f_dict.items():
        if v != '""':
            k_count = k.count("{")
            v_count = v.count("{")
            if k_count != v_count:
                print("\nwarning {}, count {} != {}\n\nmsgid {}\n\n msgstr {}\n\n".format(
                    a_file, k_count, v_count, k, v))

def gen_gtranz():
    f_result = ""
    f_list = []
    with open(f_po_file) as f_file:
        f_arr = f_file.read().split("msgid")
        for f_kvp in f_arr[1:]:
            f_key, f_val = f_kvp.split("msgstr")
            f_list.append(f_key.strip())
    f_dash_line = "\n" + "-" * 6 + "\n"
    f_result = f_dash_line.join(f_list)
    with open(f_gtranz_file, "w") as f_file:
        f_file.write(f_result)



os.system('find python -iname "*.py" | xargs xgettext '
    '--from-code=UTF-8 --default-domain={}'.format(global_pydaw_version_string))

os.system("sed --in-place '{}' --expression=s/CHARSET/UTF-8/".format(f_po_file))

gen_gtranz()

for f_file in os.listdir(f_path):
    f_locale_file = "{}/{}/{}.po".format(f_path, f_file, 
        global_pydaw_version_string)
    print(f_locale_file)
    if os.path.isfile(f_locale_file):
        os.system('msgmerge --update "{}" "{}"'.format(f_locale_file, f_po_file))
        check_escape_sequences(f_locale_file)

