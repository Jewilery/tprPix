/*
 * =================== sceneLoop_world.cpp ===================
 *                          -- tpr --
 *                                        CREATE -- 2019.04.29
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 *   正式游戏的 sceneLoop
 * ----------------------------
 */
#include "sceneLoop.h"


//-------------------- Engine --------------------//
#include "input.h"
#include "chunkBuild.h"
#include "esrc_all.h" //- 所有资源

#include "debug.h"


using namespace std::placeholders;


namespace{//-------------- namespace ------------------//


    //-- 临时数据 --
    bool  isOld_A_press {false}; //- 上一帧，A键是否按下
    bool  isOld_B_press {false}; //- 上一帧，B键是否按下
    bool  isOld_X_press {false}; //- 上一帧，X键是否按下
    bool  isOld_Y_press {false}; //- 上一帧，Y键是否按下


    bool  isNew_A_press {false};
    bool  isNew_B_press {false};
    bool  isNew_X_press {false};
    bool  isNew_Y_press {false};


    //===== funcs =====//
    void inputINS_handle_in_sceneWorld( const InputINS &_inputINS);

}//------------------ namespace: end ------------------//




/* ===========================================================
 *                prepare_for_sceneWorld
 * -----------------------------------------------------------
 * -- 在 siwth 进入 scene:world 之前，需要准备的工作
 */
void prepare_for_sceneWorld(){


    //-- 合理的流程应该是：
    //  先 读取 player 数据，还有更多 全局性数据
    //  然后根据这些数据，来 build chunks
    //
    //
    
        //--- 最简模式，仅仅生成 玩家所在的 chunk 及其周边 9 个 chunk
        //   在未来，会被 完善的 游戏存档系统 所取代
        //chunkBuild::build_9_chunks( IntVec2{ 1,1 } );

        //go_byPass();  //- 硬生产一组 Norman 实例

        //esrc::player.bind_goPtr(); //-- 务必在 go数据实例化后 再调用 --

    
    esrc::camera.set_allFPos( esrc::player.goPtr->goPos.get_currentFPos() );
    input::bind_inputINS_handleFunc( std::bind( &inputINS_handle_in_sceneWorld, _1 ) );

    switch_sceneLoop( SceneLoopType::World );
}


/* ===========================================================
 *                    sceneLoop_world
 * -----------------------------------------------------------
 */
void sceneLoop_world(){

    //--------------------------------//
    //    camera:: RenderUpdate()
    //    camera --> shader: view, projection
    //--------------------------------//
    esrc::camera.RenderUpdate();
    //--- 
    esrc::rect_shader.use_program();
    esrc::rect_shader.send_mat4_view_2_shader( esrc::camera.update_mat4_view() );
    esrc::rect_shader.send_mat4_projection_2_shader( esrc::camera.update_mat4_projection() );

    //--------------------------------//
    //           logic
    //--------------------------------//
    //-- 依据 逻辑时间循环，调用不同的 函数 --// 
    switch( esrc::logicTimeCircle.current() ){
        case 0:
            esrc::realloc_inactive_goes(); //- tmp
            break;

        case 1:
            esrc::foreach_goids_active(
                []( goid_t _goid, GameObj *_goPtr ){
                    _goPtr->LogicUpdate();
                            //-- 这么设计还是会造成 拥塞问题
                            //   大量的 go在同一帧 更新自己的 logic。
                            //   最好的办法是，分摊到 不同的帧中去...
                }
            );
            break;
        case 2:
            //esrc::camera.print_pos();
            break;
        case 3:
            //--- 定期 检查玩家所在 chunk
            //  并将需要新建的 chunks 收集到 队列中
            chunkBuild::collect_chunks_need_to_be_build_in_update();
                        // 更新中...
            break;
        case 4:
            break;
        default:
            assert(0);
    }

    //--------------------------------//
    //  每一帧，最多装配生成一个 chunk 实例
    //--------------------------------//
    chunkBuild::chunkBuild_3_receive_data_and_build_one_chunk();


    //====================================//
    //          -- RENDER --
    //    Z-Deep 深的 mesh 必须先渲染
    //====================================//

    //--- clear RenderPools:
    // *** 注意次序 ***
    esrc::renderPool_meshs.clear();
    esrc::renderPool_goMeshs_pic.clear();
    esrc::renderPool_goMeshs_shadow.clear();
    esrc::renderPool_mapSurfaces.clear();

    //------------------------//
    //       chunks
    //------------------------//
    esrc::render_chunks();

    //------------------------//
    //     mapEntSlices
    //------------------------//
    //...

    //------------------------//
    //     - shadowMeshs
    //     - picMeshs
    //------------------------//
    esrc::foreach_goids_active(
        []( goid_t _goid, GameObj *_goPtr ){
            assert( _goPtr->RenderUpdate != nullptr );
            _goPtr->RenderUpdate(); 
        }
    );

    //>>>>>>>>>>>>>>>>>>>>>>>>//
    //        draw call
    //>>>>>>>>>>>>>>>>>>>>>>>>//
    // *** 注意次序,先渲染深处的 ***
    esrc::draw_groundCanvas();
    esrc::draw_renderPool_meshs(); //- chunks
    esrc::draw_waterAnimCanvas();
    esrc::draw_renderPool_mapSurfaces();
    esrc::draw_renderPool_goMeshs_shadow();
    debug::draw_renderPool_mapEntSlices();  //-- debug 但是不在此文件中 clear
    debug::draw_renderPool_pointPics();     //-- debug 但是不在此文件中 clear
    esrc::draw_renderPool_goMeshs_pic(); 

}




namespace{//-------------- namespace ------------------//


/* ===========================================================
 *              inputINS_handle_in_sceneWorld
 * -----------------------------------------------------------
 */
void inputINS_handle_in_sceneWorld( const InputINS &_inputINS){

    //-----------------//
    //      camera
    //-----------------//
    //-- 让 camera 对其上1渲染帧 --
    //- 这会造成 camera 的延迟，但不要紧
    esrc::camera.set_targetFPos( esrc::player.goPtr->goPos.get_currentFPos() );


    //... 暂时没有 处理 剩余功能键的 代码 

    //-- 直接传递给 player
    esrc::player.handle_inputINS( _inputINS );


    //-----------------//
    //      tmp
    // 用 A 键 来增加 playerGo speed
    // 用 B 键 减速
    //-----------------//
    isNew_A_press = false;
    isNew_B_press = false;
    isNew_X_press = false;
    isNew_Y_press = false;

    if( _inputINS.check_key(GameKey::KEY_A) ){
        isNew_A_press = true;
    }
    if( _inputINS.check_key(GameKey::KEY_B) ){
        isNew_B_press = true;
    }
    if( _inputINS.check_key(GameKey::KEY_X) ){
        isNew_X_press = true;
    }
    if( _inputINS.check_key(GameKey::KEY_Y) ){
        isNew_Y_press = true;
    }




    SpeedLevel lvl = esrc::player.goPtr->move.get_speedLvl();
    //-- 有效的 节点帧 --
    if( (isOld_A_press==false) && (isNew_A_press) ){
        SpeedLevel newLvl = calc_higher_speedLvl(lvl);
        esrc::player.goPtr->move.set_speedLvl( newLvl );
            cout << " + " << static_cast<int>(newLvl) 
                << ", " << SpeedLevel_2_val(newLvl)
                << endl; 
    }
    if( (isOld_B_press==false) && (isNew_B_press) ){
        SpeedLevel newLvl = calc_lower_speedLvl(lvl);
        esrc::player.goPtr->move.set_speedLvl( newLvl );
            cout << " - " << static_cast<int>(newLvl) 
                << ", " << SpeedLevel_2_val(newLvl)
                << endl;
    }
    if( (isOld_X_press==false) && (isNew_X_press) ){

        bool is_same_fieldKey;

        const MemMapEnt *mapEntPtr = esrc::get_memMapEntPtr( esrc::player.goPtr->goPos.get_currentMPos() );

        const auto &field = esrc::atom_get_field( anyMPos_2_fieldKey(mapEntPtr->get_mpos()) );


        IntVec2 nodeMPosOff = field.get_nodeMPos() - field.get_mpos();

        cout << "mapAlti.val = " << mapEntPtr->mapAlti.val
            //<< ";   fieldKey = " << mapEntPtr->fieldKey
            << ";   nodeFieldAltiVal = " << field.get_nodeMapAlti().val
            << ";   minAltiVal = " << field.get_minMapAlti().val
            << ";   maxAltiVal = " << field.get_maxMapAlti().val
            //<< ";   nodeMPosOff = " << nodeMPosOff.x
            //<< "," << nodeMPosOff.y
            << endl;


    }
    if( (isOld_Y_press==false) && (isNew_Y_press) ){

        //-- 暂时什么都不做 ...
        
    }




    //---------------
    isOld_A_press = isNew_A_press;
    isOld_B_press = isNew_B_press;
    isOld_X_press = isNew_X_press;
    isOld_Y_press = isNew_Y_press;
}


}//------------------ namespace: end ------------------//














