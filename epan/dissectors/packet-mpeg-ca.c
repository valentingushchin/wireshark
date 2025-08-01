/* packet-mpeg-ca.c
 * Routines for MPEG2 (ISO/ISO 13818-1) Conditional Access Table (CA) dissection
 * Copyright 2012, Guy Martin <gmsoft@tuxicoman.be>
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <epan/packet.h>
#include <epan/tfs.h>
#include <wsutil/array.h>
#include "packet-mpeg-sect.h"
#include "packet-mpeg-descriptor.h"

void proto_register_mpeg_ca(void);
void proto_reg_handoff_mpeg_ca(void);

static dissector_handle_t mpeg_ca_handle;

static int proto_mpeg_ca;
static int hf_mpeg_ca_reserved;
static int hf_mpeg_ca_version_number;
static int hf_mpeg_ca_current_next_indicator;
static int hf_mpeg_ca_section_number;
static int hf_mpeg_ca_last_section_number;

static int ett_mpeg_ca;

#define MPEG_CA_RESERVED_MASK                   0xFFFFC0
#define MPEG_CA_VERSION_NUMBER_MASK             0x00003E
#define MPEG_CA_CURRENT_NEXT_INDICATOR_MASK     0x000001

static int
dissect_mpeg_ca(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void* data _U_)
{
    unsigned offset = 0, length = 0;

    proto_item *ti;
    proto_tree *mpeg_ca_tree;

    /* The TVB should start right after the section_length in the Section packet */

    col_set_str(pinfo->cinfo, COL_INFO, "Conditional Access Table (CA)");

    ti = proto_tree_add_item(tree, proto_mpeg_ca, tvb, offset, -1, ENC_NA);
    mpeg_ca_tree = proto_item_add_subtree(ti, ett_mpeg_ca);

    offset += packet_mpeg_sect_header(tvb, offset, mpeg_ca_tree, &length, NULL);
    length -= 4;

    proto_tree_add_item(mpeg_ca_tree, hf_mpeg_ca_reserved, tvb, offset, 3, ENC_BIG_ENDIAN);
    proto_tree_add_item(mpeg_ca_tree, hf_mpeg_ca_version_number, tvb, offset, 3, ENC_BIG_ENDIAN);
    proto_tree_add_item(mpeg_ca_tree, hf_mpeg_ca_current_next_indicator, tvb, offset, 3, ENC_BIG_ENDIAN);
    offset += 3;

    proto_tree_add_item(mpeg_ca_tree, hf_mpeg_ca_section_number, tvb, offset, 1, ENC_BIG_ENDIAN);
    offset += 1;

    proto_tree_add_item(mpeg_ca_tree, hf_mpeg_ca_last_section_number, tvb, offset, 1, ENC_BIG_ENDIAN);
    offset += 1;

    /* Parse all the programs */
    while (offset < length)
        offset += proto_mpeg_descriptor_dissect(tvb, pinfo, offset, mpeg_ca_tree);

    offset += packet_mpeg_sect_crc(tvb, pinfo, mpeg_ca_tree, 0, offset);

    proto_item_set_len(ti, offset);
    return tvb_captured_length(tvb);
}


void
proto_register_mpeg_ca(void)
{

    static hf_register_info hf[] = {

        { &hf_mpeg_ca_reserved, {
            "Reserved", "mpeg_ca.reserved",
            FT_UINT24, BASE_HEX, NULL, MPEG_CA_RESERVED_MASK,
                        NULL, HFILL
        } },

        { &hf_mpeg_ca_version_number, {
            "Version Number", "mpeg_ca.version",
            FT_UINT24, BASE_HEX, NULL, MPEG_CA_VERSION_NUMBER_MASK,
                        NULL, HFILL
        } },

        { &hf_mpeg_ca_current_next_indicator, {
            "Current/Next Indicator", "mpeg_ca.cur_next_ind",
            FT_BOOLEAN, 24, TFS(&tfs_current_not_yet), MPEG_CA_CURRENT_NEXT_INDICATOR_MASK,
                        NULL, HFILL
        } },

        { &hf_mpeg_ca_section_number, {
            "Section Number", "mpeg_ca.sect_num",
            FT_UINT8, BASE_DEC, NULL, 0,
                        NULL, HFILL
        } },

        { &hf_mpeg_ca_last_section_number, {
            "Last Section Number", "mpeg_ca.last_sect_num",
            FT_UINT8, BASE_DEC, NULL, 0,
                        NULL, HFILL
        } },

    };

    static int *ett[] = {
        &ett_mpeg_ca,
    };

    proto_mpeg_ca = proto_register_protocol("MPEG2 Conditional Access Table", "MPEG CA", "mpeg_ca");

    proto_register_field_array(proto_mpeg_ca, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));

    mpeg_ca_handle = register_dissector("mpeg_ca", dissect_mpeg_ca, proto_mpeg_ca);
}


void proto_reg_handoff_mpeg_ca(void)
{
    dissector_add_uint("mpeg_sect.tid", MPEG_CA_TID, mpeg_ca_handle);
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=4 tabstop=8 expandtab:
 * :indentSize=4:tabSize=8:noTabs=true:
 */
