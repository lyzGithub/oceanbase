/*
 * Copyright 2014-2021 Alibaba Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * ob_expr_uuid_short.cpp is for uuid_short function
 *
 * Date: 2021/8/20
 *
 * Authors:
 *     ailing.lcq<ailing.lcq@alibaba-inc.com>
 *
 */
#define USING_LOG_PREFIX SQL_RESV
#include "sql/engine/expr/ob_expr_uuid_short.h"
#include "observer/ob_server_struct.h"
#include "lib/time/ob_time_utility.h"

using namespace oceanbase::common;

namespace oceanbase {
namespace sql {

ObExprUuidShort::ObExprUuidShort(ObIAllocator &alloc)
    : ObFuncExprOperator(alloc, T_FUN_SYS_UUID_SHORT, N_UUID_SHORT, 0, NOT_ROW_DIMENSION)
{}

ObExprUuidShort::~ObExprUuidShort()
{}

/**
 * Note:
 * The total number of serverids over(>=) 256 will not guarantee uniqueness,
 * but we will not report an error, because this is a undefined behavior in mysql.
 * In short, users should need to know this.
 */
uint64_t ObExprUuidShort::generate_uuid_short()
{
  //                        uuid_short
  // |      <8>       |        <32>       |       <24>
  //     server_id      server_start_time   incremented_variable
  static volatile uint64_t server_id_and_server_startup_time = ((GCTX.server_id_ & 255) << 56) |
                                                               ((static_cast<uint64_t>(common::ObTimeUtility::current_time() / 1000000) << 24) &
                                                               ((static_cast<uint64_t>(1) << 56) - 1));
  uint64_t uuid_short = ATOMIC_AAF(&server_id_and_server_startup_time, 1);
  LOG_DEBUG("uuid_short generated.", K(uuid_short));
  return uuid_short;
}

int ObExprUuidShort::calc_result0(common::ObObj &result, common::ObExprCtx &expr_ctx) const
{
  int ret = OB_SUCCESS;
  UNUSED(expr_ctx);
  result.set_uint64(generate_uuid_short());
  return ret;
}

int ObExprUuidShort::cg_expr(ObExprCGCtx &expr_cg_ctx, const ObRawExpr &raw_expr, ObExpr &rt_expr) const
{
  UNUSED(raw_expr);
  UNUSED(expr_cg_ctx);
  rt_expr.eval_func_ = ObExprUuidShort::eval_uuid_short;
  return OB_SUCCESS;
}

int ObExprUuidShort::eval_uuid_short(const ObExpr &expr, ObEvalCtx &ctx, ObDatum &expr_datum)
{
  int ret = OB_SUCCESS;
  UNUSED(expr);
  UNUSED(ctx);
  expr_datum.set_uint(generate_uuid_short());
  return ret;
}

}  // namespace sql
}  // namespace oceanbase
