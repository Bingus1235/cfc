#!/usr/bin/env python3
# Copyright (c) 2023 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at https://mozilla.org/MPL/2.0/.
"""Creates symsrv directory structure containing exe/dll/pdb files."""

import configparser
import errno
import argparse
import os
import re
import sys
import subprocess

from datetime import datetime
from shutil import rmtree, copy

root_dir = os.path.join(os.path.dirname(__file__), *[os.pardir] * 3)
sys.path.append(os.path.join(root_dir, 'tools', 'symsrc'))

# pylint: disable=import-error,wrong-import-position
from pdb_fingerprint_from_img import GetPDBInfoFromImg


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--installer-config',
        required=True,
        help='Installer config that contains files to process.',
    )
    parser.add_argument(
        '--additional-files',
        nargs='*',
        help='Additional exe/dll files to process.',
    )
    parser.add_argument(
        '--build-dir',
        required=True,
        help='The build output directory.',
    )
    parser.add_argument(
        '--symbols-dir',
        required=True,
        help='Directory to output all files for symbol server (exe, dll, pdb).',
    )
    parser.add_argument(
        '--pdb-only-symbols-dir',
        help='Directory to output only pdb files.',
    )
    parser.add_argument(
        '--clear',
        default=False,
        action='store_true',
        help='Clear the symbol directories before writing new symbols.',
    )
    parser.add_argument(
        '-v',
        '--verbose',
        action='store_true',
        help='Print verbose status output.',
    )

    args = parser.parse_intermixed_args()

    if args.clear:
        try:
            rmtree(args.symbols_dir)
            if args.pdb_only_symbols_dir:
                rmtree(args.pdb_only_symbols_dir)
        except:  # pylint: disable=bare-except
            pass

    images_with_pdbs = get_images_with_pdbs(args)
    for image_path in images_with_pdbs:
        process_image(args, image_path)


def get_images_with_pdbs(args):
    images_with_pdbs = set()

    def add_if_has_pdb(file):
        file = os.path.join(args.build_dir, file)
        if not os.path.exists(file):
            return
        pdb_file = file + '.pdb'
        if not os.path.exists(pdb_file):
            return
        images_with_pdbs.add(file)

    installer_config = configparser.ConfigParser()
    installer_config.optionxform = str  # Preserve string case.
    installer_config.read(args.installer_config)
    for group in installer_config:
        if group == 'GOOGLE_CHROME':
            # Skip Google Chrome-only files.
            continue

        for file in installer_config[group]:
            add_if_has_pdb(file)

    if args.additional_files:
        for file in args.additional_files:
            add_if_has_pdb(file)

    return [*images_with_pdbs]


def process_image(args, image_path):
    assert os.path.isabs(image_path)
    start_time = datetime.utcnow()
    if args.verbose:
        print(f'Processing {image_path}')

    image_pdb_fingerprint, pdb_path = GetPDBInfoFromImg(image_path)
    if args.verbose:
        print(f'Image PDB fingerprint {image_pdb_fingerprint}')

    assert os.path.isabs(pdb_path)
    pdb_fingerprint = get_pdb_fingerprint(pdb_path)
    if args.verbose:
        print(f'      PDB fingerprint {pdb_fingerprint}')

    if pdb_fingerprint != image_pdb_fingerprint:
        raise ValueError(
            f"Image PDB fingerprint doesn't match PDB fingerprint:\n"
            f"  {image_pdb_fingerprint} : {image_path}\n"
            f"  {pdb_fingerprint} : {pdb_path}")

    copy_symbol(args.symbols_dir, image_path, image_pdb_fingerprint)
    copied_pdb_path = copy_symbol(args.symbols_dir, pdb_path, pdb_fingerprint)

    # Inject source server information into PDBs to allow VS/WinDBG
    # automatically fetch sources from GitHub.
    #
    # Note: this will modify PDB header age, but not DBI age which allows
    # debuggers to properly work with the modified PDB. More info in comments:
    # https://randomascii.wordpress.com/2011/11/11/source-indexing-is-underused-awesomeness/
    subprocess.check_call([
        'vpython3.bat',
        os.path.join(root_dir, 'tools', 'symsrc', 'source_index.py'),
        '--build-dir',
        args.build_dir,
        '--toolchain-dir',
        'c:\\Program Files (x86)',
        copied_pdb_path,
    ])

    if args.pdb_only_symbols_dir:
        copy_symbol(args.pdb_only_symbols_dir, copied_pdb_path, pdb_fingerprint)

    if args.verbose:
        elapsed = datetime.utcnow() - start_time
        print(f'Copied symbols for {image_path}: elapsed time '
              f'{elapsed.total_seconds()} seconds')


def get_pdb_fingerprint(pdb_path):
    assert os.path.isabs(pdb_path)
    llvm_pdbutil_path = os.path.join(root_dir, 'third_party', 'llvm-build',
                                     'Release+Asserts', 'bin',
                                     'llvm-pdbutil.exe')

    stdout = subprocess.check_output(
        [llvm_pdbutil_path, 'dump', '--summary', pdb_path], encoding='oem')

    age_pattern = r'  Age: (.*)'
    guid_pattern = r'  GUID: {(.*)}'

    age_match = None
    guid_match = None
    for line in stdout.splitlines():
        if not age_match:
            age_match = re.match(age_pattern, line)
        if not guid_match:
            guid_match = re.match(guid_pattern, line)

    if not age_match or not guid_match:
        raise RuntimeError(
            f'PDB info not matched in llvm-pdbutil output for {pdb_path}:\n'
            f'{stdout}')

    return guid_match.group(1).replace('-', '') + age_match.group(1)


def copy_symbol(symbols_dir, symbol_path, symbol_fingerprint):
    symbol_name = os.path.basename(symbol_path)
    symbol_dir = os.path.join(symbols_dir, symbol_name, symbol_fingerprint)
    mkdir_p(symbol_dir)
    dest_symbol_path = os.path.join(symbol_dir, symbol_name)
    copy(symbol_path, dest_symbol_path)
    return dest_symbol_path


def mkdir_p(path):
    '''Simulates mkdir -p.'''
    try:
        os.makedirs(path)
    except OSError as e:
        if e.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise


if __name__ == '__main__':
    main()
