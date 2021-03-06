/**
 * (C) 2010-2011 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * ob_rpc_stub.h for rpc among chunk server, update server and
 * root server.
 *
 * Authors:
 *    qushan <qushan@alipay.com>
 *
 */
#ifndef OCEANBASE_COMMON_CTRL_RPC_STUB_H_
#define OCEANBASE_COMMON_CTRL_RPC_STUB_H_

#include "ob_server.h"
#include "ob_rpc_stub.h"
#include "common/location/ob_tablet_location_list.h"
#include "sql/ob_physical_plan.h"
#include "sql/ob_ups_result.h"
#include "common/ob_transaction.h"
namespace oceanbase
{
  namespace sql
  {
    class ObPhysicalPlan;
    class ObUpsResult;
  };
  namespace common
  {
    class ObString;
    class ObScanner;
    class ObMutator;
    class ObGetParam;
    class ObScanParam;
    class ObDataBuffer;
    class ObOperateResult;
    class ObSchemaManagerV2;
    class ObClientManager;
    class ThreadSpecificBuffer;
    class TableSchema;
    class ObUpsList;
    class ObStrings;
    class ObTabletReportInfoList;
    class ObTabletLocation;
    class ObiRole;
    class AlterTableSchema;

    // this class encapsulates network rpc interface as bottom layer,
    // and it only take charge of "one" rpc call.
    // if u need other operational work, please use rpc_proxy for interaction
    class ObGeneralRpcStub : public ObDataRpcStub
    {
      public:
        ObGeneralRpcStub();
        virtual ~ObGeneralRpcStub();

      public:
        // register to root server as a merge server by rpc call
        // param  @timeout  action timeout
        //        @root_server root server addr
        //        @merge_server merge server addr
        //        @is_merger merge server status
        int register_server(const int64_t timeout, const common::ObServer & root_server,
            const common::ObServer & server, const bool is_merger, int32_t &status, const char* server_version) const;

        // register to root server as a merge server by rpc call
        // param  @timeout  action timeout
        //        @root_server root server addr
        //        @merge_server merge server addr
        //        @is_merger merge server status
        int register_merge_server(const int64_t timeout,
                                  const common::ObServer & root_server,
                                  const common::ObServer & server,
                                  const int32_t sql_port, int32_t &status, const char* server_version) const;

        // heartbeat to root server for alive
        // param  @timeout  action timeout
        //        @root_server root server addr
        //        @merge_server merge server addr
        //        @server_role server role
        int heartbeat_server(const int64_t timeout, const common::ObServer & root_server,
            const common::ObServer & merge_server, const common::ObRole server_role) const;

        // get update server vip addr through root server rpc call
        // param  @timeout  action timeout
        //        @root_server root server addr
        //        @update_server output server addr
        int find_server(const int64_t timeout, const common::ObServer & root_server,
            common::ObServer & update_server) const;


        // get update server addr through root server rpc call
        // param  @timeout  action timeout
        //        @root_server root server addr
        //        @update_server output server addr
        //        @for_merge if get update server for daily merge
        int fetch_update_server(const int64_t timeout, const common::ObServer & root_server,
            common::ObServer & update_server, bool for_merge = false) const;

        // get update server list for read through root server rpc call
        // param  @timeout  action timeout
        //        @root_server root server addr
        //        @ups_list output server addr list info
        int fetch_server_list(const int64_t timeout, const common::ObServer & root_server,
            common::ObUpsList & server_list) const;

        // get frozen time from update server according to frozen version
        // param  @timeout  action timeout
        //        @update_server output server addr
        //        @frozen_version frozen version to query
        //        @frozen_time frozen time which the frozen version creates
        int fetch_frozen_time(const int64_t timeout, common::ObServer & update_server,
            const int64_t frozen_version, int64_t& frozen_time) const;

        // get tables schema info through root server rpc call
        // param  @timeout  action timeout
        //        @root_server root server addr
        //        @timestamp  fetch cmd input param
        //        @schema fetch cmd output schema data
        int fetch_schema(const int64_t timeout, const common::ObServer & root_server,
            const int64_t timestamp, const bool only_core_tables, common::ObSchemaManagerV2 & schema) const;

        // get tables schema newest version through root server rpc call
        // param  @timeout  action timeout
        //        @root_server root server addr
        //        @timestamp output new version
        int fetch_schema_version(const int64_t timeout, const common::ObServer & root_server,
            int64_t & timestamp) const;

        // get tablet location info through root server rpc call
        // param  @timeout  action timeout
        //        @root_server root server addr
        //        @root_table root table name
        //        @table_id look up table id
        //        @row_key look up row key
        //        @scanner scaned tablets location result set
        int fetch_tablet_location(const int64_t timeout, const common::ObServer & root_server,
            const uint64_t root_table_id, const uint64_t table_id,
            const common::ObRowkey & row_key, common::ObScanner & scanner) const;

        // apply mutator through rpc call
        // param  @timeout rpc timeout
        //        @server server addr
        //        @mutator parameter
        //        @has_data return data
        //        @scanner return result
        int mutate(const int64_t timeout, const common::ObServer & server,
            const common::ObMutator & mutate_param, const bool has_data, common::ObScanner & scanner) const;
        int ups_apply(const int64_t timeout, const common::ObServer & server, 
            const common::ObMutator & mutate_param) const;

        // reload config for server it self
        // param  @timeout rpc timeout
        //        @ merge_server  server addr
        //        @ filename config file name to reload
        int reload_self_config(const int64_t timeout, const common::ObServer & merge_server, const char *filename) const;

        // create table
        int create_table(const int64_t timeout, const common::ObServer & root_server,
            bool if_not_exists, const common::TableSchema & table_schema) const;
        // drop table
        int drop_table(const int64_t timeout, const common::ObServer & root_server,
            bool if_exists, const common::ObStrings & tables) const;
        // alter table
        int alter_table(const int64_t timeout, const common::ObServer & root_server,
            const common::AlterTableSchema & alter_schema) const;

        /*
         * Report tablets to RootServer, report finishes at has_more flag is off
         * @param tablets TabletReportInfoList to be reported
         * @param has_more true when there are remaining TabletS to be reported,
         *                 false when all Tablets has been reported
         * @return OB_SUCCESS when successful, otherwise failed
         */
        int report_tablets(const int64_t timeout, const ObServer & root_server,
            const ObServer &client_server,  const ObTabletReportInfoList& tablets,
            const int64_t time_stamp, bool has_more);

        int delete_tablets(const int64_t timeout, const ObServer & root_server,
            const ObServer &client_server, const common::ObTabletReportInfoList& tablets);

        /*
         *  notify dest_server to load tablet
         * @param [in] dest_server    migrate destination server
         * @param [in] range          migrated tablet's range
         * @param [in] size           path array size
         * @param [in] tablet_version migrate tablet's data version
         * @param [in] path           sstable file path array
         * @param [in] dest_disk_no   destination disk no
         */
        int dest_load_tablet(
            const int64_t timeout,
            const common::ObServer &dest_server,
            const common::ObNewRange &range,
            const int32_t dest_disk_no,
            const int64_t tablet_version,
            const int64_t tablet_seq_num,
            const uint64_t crc_sum,
            const int64_t size,
            const char (*path)[common::OB_MAX_FILE_NAME_LENGTH]);


        /*
         * notify rootserver migrate tablet is over.
         * @param [in] range migrated tablet's range
         * @param [in] src_server migrate source server
         * @param [in] dest_server migrate destination server
         * @param [in] keep_src =true means copy otherwise move
         * @param [in] tablet_version migrated tablet's data version
         */
        int migrate_over(
            const int64_t timeout,
            const ObServer & root_server,
            const ObNewRange &range,
            const ObServer &src_server,
            const ObServer &dest_server,
            const bool keep_src,
            const int64_t tablet_version,
            const int64_t tablet_seq_num);

        /*
         * report capacity info of chunkserver for load balance.
         * @param [in] server ip of chunkserver
         * @param [in] capacity total capacity of all disks.
         * @param [in] used total size been used.
         */
        int report_capacity_info(
            const int64_t timeout, const ObServer & root_server,
            const ObServer &server, const int64_t capacity, const int64_t used);

        /*
         * @brief source server need to know location of destination server
         * (like disk_no, path) before migrate a tablet.
         * @param [in] occupy_size migrate sstable files occupy size at disk.
         * @param [out] dest_disk_no destination disk no for store migrate sstable files.
         * @param [out] dest_path destination path for store migrate sstable files.
         */
        int get_migrate_dest_location( const int64_t timeout, const ObServer & root_server,
            const int64_t occupy_size, int32_t &dest_disk_no, common::ObString &dest_path);

        // get last frozen memtable version on updateserver;
        int get_last_frozen_memtable_version(const int64_t timeout,
            const ObServer & root_server, int64_t &last_version) const;

        // find newest version of another replicas
        int get_tablet_info(const int64_t timeout, const ObServer & root_server,
            const common::ObSchemaManagerV2& schema,
            const uint64_t table_id, const common::ObNewRange& range,
            ObTabletLocation location [], int32_t& size);

        int merge_tablets_over(const int64_t timeout, const ObServer & root_server,
            const common::ObTabletReportInfoList& tablet_list, const bool is_merge_succ);

        // get row info through rpc call
        // param  @timeout  action timeout
        //        @server server addr
        //        @get_param get parameter
        //        @scanner  return result
        int get(const int64_t timeout, const common::ObServer & server,
            const common::ObGetParam & get_param, common::ObScanner & scanner) const;

        // sort the locationlist asc by distance between input server ip
        // retry another server in the list when the previous one failed
        // waring: timeout is every rpc call time
        // param  @timeout  action timeout
        //        @list sorted location list
        //        @get_param get parameter
        //        @succ_addr get succ response server
        //        @scanner return result
        //        @update_list location list modified status
        int get(const int64_t timeout, ObTabletLocationList & list,
            const common::ObGetParam & get_param, ObTabletLocationItem & succ_addr,
            common::ObScanner & scanner, bool & update_list) const;

        // scan row info through rpc call
        // param  @timeout  action timeout
        //        @server server addr
        //        @scan_param scan parameter
        //        @scanner  return result
        int scan(const int64_t timeout, const common::ObServer & server,
            const common::ObScanParam & scan_param, common::ObScanner & scanner) const;

        // sort the locationlist asc by distance between input server ip
        // retry another server in the list when the previous one failed
        // waring: timeout is every rpc call time
        // param  @timeout  action timeout
        //        @server input server addr
        //        @list sorted location list
        //        @scan_param scan parameter
        //        @succ_addr scan succ response server
        //        @scanner return result
        //        @update_list location list modified status
        int scan(const int64_t timeout, ObTabletLocationList & list,
            const common::ObScanParam & scan_param, ObTabletLocationItem & succ_addr,
            common::ObScanner & scanner, bool & update_list) const;

        int load_bypass_sstables_over(const int64_t timeout, const ObServer & root_server,
          const ObServer& self, const common::ObTableImportInfoList& table_list, const bool is_load_succ);

        int delete_table_over(const int64_t timeout, const ObServer & root_server,
          const ObServer& self, const uint64_t table_id, const bool is_delete_succ);

        // get rootserver's obi role
        int get_obi_role(const int64_t timeout_us, const common::ObServer& root_server, common::ObiRole &obi_role) const;
        // get master ups info from root server
        int get_master_ups_info(const int64_t timeout_us, const ObServer& root_server, ObServer &master_ups) const;
        int ups_plan_execute(const int64_t timeout, const ObServer & ups,
                             const sql::ObPhysicalPlan &plan, sql::ObUpsResult &result) const;
        int ups_start_trans(const int64_t timeout, const ObServer & ups,
                            const ObTransReq &req, ObTransID &trans_id) const;
        int ups_end_trans(const int64_t timeout, const ObServer & ups,
                          const ObEndTransReq &req) const;
        // request ms to execute sql with no result set
        int execute_sql(const int64_t timeout, const ObServer & ms, const ObString &sql_str) const;
        /* get master obi rootserver address */
        int get_master_obi_rs(const int64_t timeout, const ObServer &rs, ObServer &master_obi_rs) const;
      protected:
        // default cmd version
        static const int32_t DEFAULT_VERSION = 1;
        // for heartbeat cmd version
        static const int32_t NEW_VERSION = 2;
    };

  }
}

#endif // OCEANBASE_COMMON_CTRL_RPC_STUB_H_
