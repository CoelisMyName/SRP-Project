package com.scut.ui

import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.RatingBar
import android.widget.TextView
import com.scut.MainActivity
import com.scut.databinding.FragmentEssBinding
import com.scut.R

/**
 * A simple [Fragment] subclass.
 * For ESS self-test.
 * create an instance of this fragment.
 */
class EssFragment : Fragment() {

    private lateinit var mBinding: FragmentEssBinding
    private lateinit var mScores: Array<Pair<RatingBar, TextView>>

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?,
                              savedInstanceState: Bundle?): View {
        this.mBinding = FragmentEssBinding.inflate(inflater, container, false)
        // 绑定返回按钮
        this.mBinding.goBack.setOnClickListener { (activity as MainActivity).popFragment() }
        // 初始化快捷数组
        this.mScores = arrayOf(
                Pair(this.mBinding.question1, this.mBinding.response1),
                Pair(this.mBinding.question2, this.mBinding.response2),
                Pair(this.mBinding.question3, this.mBinding.response3),
                Pair(this.mBinding.question4, this.mBinding.response4),
                Pair(this.mBinding.question5, this.mBinding.response5),
                Pair(this.mBinding.question6, this.mBinding.response6),
                Pair(this.mBinding.question7, this.mBinding.response7),
                Pair(this.mBinding.question8, this.mBinding.response8)
        )
        // 绑定回调
        for (p in this.mScores) {
            p.first.setOnRatingBarChangeListener { bar, r, _ ->
                if (r < 1f) {
                    bar.rating = 1f
                }
                allUpdate()
            }
        }
        // 首次初始化
        this.allUpdate()
        return this.mBinding.root
    }

    fun allUpdate() {
        var score = 0
        for (pair in mScores) {
            val r = pair.first.rating.toInt() - 1
            pair.second.text = getString(when (r) {
                0 -> R.string.ess_test_choice_0
                1 -> R.string.ess_test_choice_1
                2 -> R.string.ess_test_choice_2
                3 -> R.string.ess_test_choice_3
                else -> R.string.unexcept
            })
            score += r
        }
        mBinding.sumPoint.text = score.toString()
        mBinding.result.text = getString(when (score) {
            in 0.. 6 -> R.string.ess_test_result_1
            in 7.. 10 -> R.string.ess_test_result_2
            in 11.. 16 -> R.string.ess_test_result_3
            in 17.. 24 -> R.string.ess_test_result_4
            else -> R.string.unexcept
        })

    }
}