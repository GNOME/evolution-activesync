/*
 * ActiveSync DBus dæmon
 *
 * Copyright © 2011 Intel Corporation.
 *
 * Authors: Mobica Ltd. <www.mobica.com>
 *
 * This file is provided under a dual Apache/LGPLv2.1 licence.  When
 * using or redistributing this file, you may do so under either
 * licence.
 *
 *
 * LGPLv2.1 LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later
 *   version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free
 *   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301 USA
 *
 *
 * APACHE LICENCE SUMMARY
 *
 *   Copyright © Intel Corporation, dates as above.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef ACTIVESYNCD_COMMON_DEFS_H
#define ACTIVESYNCD_COMMON_DEFS_H

/*
#define EAS_SERVICE_NAME        "org.meego.eas.daemon"
#define EAS_SERVICE_OBJECT_PATH "/EAS"
#define EAS_SERVICE_INTERFACE   "org.meego.Eas"
*/

#define EAS_SERVICE_NAME        "org.meego.activesyncd"

#define EAS_SERVICE_SYNC_OBJECT_PATH "/EasSync"
#define EAS_SERVICE_SYNC_INTERFACE   "org.meego.activesyncd.EasSync"

#define EAS_SERVICE_COMMON_OBJECT_PATH 	 "/EasCommon"
#define EAS_SERVICE_COMMON_INTERFACE   "org.meego.activesyncd.EasCommon"


#define EAS_SERVICE_MAIL_INTERFACE   "org.meego.activesyncd.EasMail"
#define EAS_SERVICE_MAIL_OBJECT_PATH	 "/EasMail"

#define EAS_SERVICE_TEST_INTERFACE    "org.meego.activesyncd.EasTest"
#define EAS_SERVICE_TEST_OBJECT_PATH  "/EasTest"
#define EAS_MAIL_SIGNAL_PROGRESS "mail_operation_progress"

#endif /* ACTIVESYNCD_COMMON_DEFS_H */

