   margin: 0;                    }                                        .<%=p%>_widget .<%=p%>_switch > input + label {                        cursor: pointer;                        position: absolute;                        top: 0;                        left: 0;                        right: 0;                        bottom: 0;                        background: rgba(0,0,0,1);                        border: 2px solid rgba(255,255,255,.3);                        border-radius: 100px;                        -webkit-transition: 400ms all;                           -moz-transition: 400ms all;                                transition: 400ms all;                    }                                        .<%=p%>_widget .<%=p%>_switch > input:checked + label {                        background: rgba(0,0,0,.3);                        border-color: transparent;                        -webkit-transition: 250ms all;                           -moz-transition: 250ms all;                                transition: 250ms all;                    }                                        .<%=p%>_widget .<%=p%>_switch > input + label:before {                        content: "";                        transition: 300ms all;                        position: absolute;                        left: 0;                        top: 0;                        display: block;                        width: 24px;                        height: 24px;                        border-radius: 40px;                        background-color: #ffffff;                        background-position: center center;                        background-repeat: no-repeat;                        background-size: 5px;                        -webkit-box-shadow: 0 0 12px 0 rgba(0,0,0,.06), 0 0 0 0 rgba(0,0,0,.06), 0 6px 10px 0 rgba(0,0,0,.15), 0 0 2px 0 rgba(0,0,0,.07), 0 4px 6px 0 rgba(0,0,0,.06), 0 1px 1px 0 rgba(0,0,0,.11);                        -moz-box-shadow: 0 0 12px 0 rgba(0,0,0,.06), 0 0 0 0 rgba(0,0,0,.06), 0 6px 10px 0 rgba(0,0,0,.15), 0 0 2px 0 rgba(0,0,0,.07), 0 4px 6px 0 rgba(0,0,0,.06), 0 1px 1px 0 rgba(0,0,0,.11);                        box-shadow: 0 0 12px 0 rgba(0,0,0,.06), 0 0 0 0 rgba(0,0,0,.06), 0 6px 10px 0 rgba(0,0,0,.15), 0 0 2px 0 rgba(0,0,0,.07), 0 4px 6px 0 rgba(0,0,0,.06), 0 1px 1px 0 rgba(0,0,0,.11);                    }                                        .<%=p%>_widget .<%=p%>_switch > input + label > span {                        position: absolute;                        z-index: 3;                        color: <%= widgetStyle.accentColor %>;                        background-color: transparent !important;                        border: 0 !important;                        width: 13px !important;                        height: 14px !important;                        left: 6px;                        top: 5px;                        margin: 0 !important;                        opacity: 0;                        font-size: 14px;                        pointer-events: none !important;                        -webkit-transition: 250ms all;                           -moz-transition: 250ms all;                                transition: 250ms all;                    }                                        .<%=p%>_widget .<%=p%>_switch > input:checked + label:before {                        left: 18px;                    }                                        .<%=p%>_widget .<%=p%>_switch > input:checked + label > span {                        left: 24px;      