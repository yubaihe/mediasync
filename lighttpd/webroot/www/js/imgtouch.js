function touchScale(seletor) {
    // 参数one是为了区分屏幕上现在是一根手指还是两根手指，因为在滑动的时候两根手指先离开一根会触发一根手指移动图片位置这个方法
      var one = false, 
          $touch = $(seletor),
          imgLeft = parseInt($touch.css('left')),
          imgTop = parseInt($touch.css('top')),
          originalWidth = $touch.width(),
          originalHeight = $touch.height(),
          baseScale = parseFloat(originalWidth/originalHeight);
      var store = {
        scale: 1
      };
      function siteData(name) {
          imgLeft = parseInt(name.css('left'));
          imgTop = parseInt(name.css('top'));
      }
      // 获取坐标之间的距离
      function getDistance(start, stop) {
        return Math.hypot(stop.x - start.x, stop.y - start.y);
      }
      // $(seletor).parent().selector让我们可以选择类，不一定要指定id
      // $('#'+).on('touchstart touchmove touchend', function(event){
      $(document).on('touchstart touchmove touchend', $(seletor).parent().selector, function(event){
          var $me = $(seletor),
              touch1 = event.originalEvent.targetTouches[0],  // 第一根手指touch事件
              touch2 = event.originalEvent.targetTouches[1],  // 第二根手指touch事件
              fingers = event.originalEvent.touches.length;   // 屏幕上手指数量
          //手指放到屏幕上的时候，还没有进行其他操作
          if (event.type == 'touchstart') {
              // 阻止事件冒泡
              event.preventDefault();
              // 第一个触摸点的坐标
              store.pageX = touch1.pageX;
              store.pageY = touch1.pageY;
              if (fingers == 2) {
                  store.pageX2 = touch2.pageX;
                  store.pageY2 = touch2.pageY;
                  store.moveable = true;
                  one = false;
              }
              else if (fingers == 1) {
                  one = true;
              }
              store.originScale = store.scale || 1;
          }
          //手指在屏幕上滑动
          else if (event.type == 'touchmove') {
              event.preventDefault();
              if (fingers == 2) {
                  if (!store.moveable) {
                      return;
                  }
                  // 双指移动
                  if (touch2) {
                    if(!store.pageX2) {
                      store.pageX2 = touch2.pageX;
                    }
                    if(!store.pageY2) {
                      store.pageY2 = touch2.pageY;
                    }
                  }else{return
                  }
                  // 双指缩放比例计算:originScale/scale=初始距离/后来距离，即：scale=originScale*（后来距离/初始距离)
                  var zoom = getDistance({
                      x: touch1.pageX,
                      y: touch1.pageY
                  }, {
                      x: touch2.pageX,
                      y: touch2.pageY
                  }) /
                  getDistance({
                      x: store.pageX,
                      y: store.pageY
                  }, {
                      x: store.pageX2,
                      y: store.pageY2
                  });
                  var newScale = store.originScale * zoom
                  // 最大缩放比例限制
                  if (newScale > 3) {
                      newScale = 3;
                  }
                  // 记住当前缩放值
                  store.scale = newScale
                  // 图片缩放效果
                  $me.css({
                      'left' : imgLeft,
                      'top' : imgTop,
                      // 1.通过修改宽高缩放图片
                      'width' : originalWidth*newScale,
                      'height' : (originalWidth*newScale)/baseScale,
                      // 2.通过修改transform缩放图片，transform-origin作用中心点
                      // 'transform': 'scale('+newScale+')',
                      // 'transform-origin': '0 0' 
                  });
              }else if (fingers == 1) {
                  if (one) {
                      $me.css({
                          'left' : imgLeft + (touch1.pageX - store.pageX),
                          'top' : imgTop + (touch1.pageY - store.page)
                      });
                  }
              }
          }
          //手指移开屏幕
          else if (event.type == 'touchend') {
             store.moveable = false;
             delete store.pageX2;
             delete store.pageY2;
          }
      });
  };