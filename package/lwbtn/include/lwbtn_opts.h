/**
 * @file lwbtn_opts.h
 * @brief FLY default lwbtn options header
 *
 * FLY keeps no project-specific values here: every knob is injected by
 * Kconfig through PUBLIC LWBTN_CFG_* compile definitions on the `lwbtn`
 * target (package/lwbtn/CMakeLists.txt). lwbtn_opt.h wraps each option in
 * #ifndef, so those -D values win over the upstream defaults below.
 *
 * The guard turns "included lwbtn.h without linking the lwbtn target" into a
 * build error instead of a silent struct-layout mismatch. It cannot detect a
 * hand-written override with a *different value*; do not add one.
 *
 * LWBTN_CFG_GET_STATE_MODE must stay at the upstream default
 * LWBTN_GET_STATE_MODE_CALLBACK (lwbtn_opt.h): consumers drive button state
 * through the get_state callback. Setting MANUAL would silently ignore it.
 */

#ifndef LWBTN_OPTS_HDR_H
#define LWBTN_OPTS_HDR_H

#if !defined(LWBTN_CFG_USE_CLICK) || !defined(LWBTN_CFG_USE_KEEPALIVE) \
    || !defined(LWBTN_CFG_TIME_DEBOUNCE_PRESS) || !defined(LWBTN_CFG_TIME_KEEPALIVE_PERIOD)
#error "lwbtn: link the FLY lwbtn target (its PUBLIC LWBTN_CFG_* definitions are required)"
#endif

#endif /* LWBTN_OPTS_HDR_H */
