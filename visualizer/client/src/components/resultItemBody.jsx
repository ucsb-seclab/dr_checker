
import React from 'react';
import PropTypes from 'prop-types';
import { CardContent } from 'material-ui/Card';
import Collapse from 'material-ui/transitions/Collapse';
import Divider from 'material-ui/Divider';
import SourcefileResult from './sourcefileResult.jsx';

/**
 * This class represent the body of the resultItem.
 * It encapsulatesvarious subcomponents responsble to show for example
 * the highlighted source code, the summary of the analysis relative to this
 * this function, etc...
 */
class ResultItemBody extends React.PureComponent {
  render() {
    return (
      <Collapse in={this.props.expanded} transitionDuration="auto" unmountOnExit>
        <Divider />
        <CardContent>
          {
            this.props.alreadyFetched ?
              (Object.keys(this.props.warnings).map(key => (
                <SourcefileResult
                  key={key}
                  filename={key}
                  warnings={this.props.warnings[key]}
                />
              )))
              :
              (<h4>Loading...</h4>)
          }
        </CardContent>
      </Collapse>
    );
  }
}

ResultItemBody.propTypes = {
  // toggle card collapse
  expanded: PropTypes.bool.isRequired,
  // list of warnings grouped by filename
  warnings: PropTypes.object.isRequired,
  alreadyFetched: PropTypes.bool.isRequired,
};

export default ResultItemBody;
