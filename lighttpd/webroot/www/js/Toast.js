var g_Toast = null;
function Toast(tips)
{
    UnSetToast();
    $(document).dialog({
        type : 'notice',
        infoText: tips,
        autoClose: 1000,
        position: 'bottom'  // center: 居中; bottom: 底部
    });
}

function ToastSuccess(tips)
{
    UnSetToast();
    $(document).dialog({
        type : 'toast',
        overlayShow: true,
        infoIcon: '../img/success.png',
        infoText: tips,
        autoClose: 1000
    });
}

function ToastError(tips)
{
    UnSetToast();
    $(document).dialog({
        type : 'toast',
        overlayShow: true,
        infoIcon: '../img/fail.png',
        infoText: tips,
        autoClose: 1000
    });
}

function SetToast(tips)
{
    if(g_Toast == null)
    {
        g_Toast = $(document).dialog({
            type : 'toast',
            infoIcon: '../img/loading.gif',
            infoText: tips,
        });

        $(".dialog-overlay").addClass("dialog-overlay1");
    }
    else
    {
        g_Toast.infoText = tips;
        $(document).find(".info-text").eq(0).html(tips);

        // g_Toast.update({

        //     infoIcon: '../img/loading.gif',

        //     infoText: tips

        // });
    }
}

function UnSetToast()
{
    if(g_Toast != null)
    {
        $(".dialog-overlay").removeClass("dialog-overlay1");
        g_Toast.close();
        g_Toast = null;
    }
}

function ShowConfirm(title, content, tag, sureBtn, cancelBtn)
{
    $(document).dialog({
        type : 'confirm',
        titleText:title,
        content: content,
        style: 'ios',
        onClickConfirmBtn: function()
        {
            sureBtn(tag);
        },
        onClickCancelBtn : function(){
            cancelBtn(tag);
        }
    });
}


function ShowConfirm(title, content, tag, sureBtnText, sureBtn, cancelBtnText, cancelBtn)
{
    $(document).dialog({
        type : 'confirm',
        titleText:title,
        content: content,
        style: 'ios',
        buttons: [          
            {
                name: cancelBtnText,
                callback: function() {
                    cancelBtn(tag);
                }
            },
            {
                name: sureBtnText,
                callback: function() {
                    sureBtn(tag);
                }
            }
        ]
    });
}


