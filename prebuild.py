# Build version header writer
# S.J.Morley, Reified Ltd.

# IMPORTANT: This file is auto-generated prior to every build. 
# Do not edit.

import os
import datetime

def write_version_file(filename):
    build_version = '0.1.0-alpha'
    now = datetime.datetime.now()
    build_date = now.strftime('%Y-%m-%d')
    build_time = now.strftime('%H:%M:%S')
    with open(filename, 'w') as f:
        f.write(f'#define BUILD_VERSION "{build_version}"\n')
        f.write(f'#define BUILD_DATE "{build_date}"\n')
        f.write(f'#define BUILD_TIME "{build_time}"\n')

def main():
    filename = 'include/_Build/Version.h'
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    write_version_file(filename)

main()